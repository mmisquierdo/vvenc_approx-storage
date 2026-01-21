#include "ApproxInter.h"

BufferRange::BufferRange(uint8_t * const initialAddress, uint8_t const * const endAddress) : 
			m_initialAddress(initialAddress), m_endAddress(endAddress), m_bufferId(-1), m_configurationId(-1), m_dataSizeInBytes(2) {}

BufferRange::BufferRange(uint8_t * const initialAddress, uint8_t const * const endAddress, const int64_t bufferId, const int64_t configurationId, const uint32_t dataSizeInBytes) :
			m_initialAddress(initialAddress), m_endAddress(endAddress), m_bufferId(bufferId), m_configurationId(configurationId), m_dataSizeInBytes(dataSizeInBytes) {}

AllocatedBuffersSet ApproxInter::allocatedBuffers{};
//AllocatedBuffersSet ApproxInter::unmarkedBuffers{};
std::mutex ApproxInter::allocatedBuffersMutex;

int ApproxInter::lastFuncId = -1;

uint64_t ApproxInter::Take::CurrentTake = ApproxInter::Take::Precise;


//FME_BEST_MV_COST_RECALC
uint32_t ApproxInter::fme_uiDirecBest = 0;

//int64_t ApproxInter::BufferId::FME_FILT			[vvenc::LUMA_INTERPOLATION_FILTER_SUB_SAMPLE_POSITIONS_SIGNAL][vvenc::LUMA_INTERPOLATION_FILTER_SUB_SAMPLE_POSITIONS_SIGNAL][vvenc::ComponentID::MAX_NUM_COMP];
//int64_t ApproxInter::BufferId::FME_FILT_TEMP	[vvenc::LUMA_INTERPOLATION_FILTER_SUB_SAMPLE_POSITIONS_SIGNAL][vvenc::ComponentID::MAX_NUM_COMP];

#if PRINT_COST
	double ApproxInter::bestTempCost = 666;
#endif

void ApproxInter::MarkBuffer(void * const initialAddress, void const * const endAddress, const int64_t bufferId, const int64_t configurationId, const uint32_t dataSizeInBytes) {
	ApproxInter::MarkBuffer(BufferRange((uint8_t*) initialAddress, (uint8_t*) endAddress, bufferId, configurationId, dataSizeInBytes));
}

void ApproxInter::RemarkBuffer(void * const initialAddress/*, void const * const endAddress*/, const int64_t bufferId, const int64_t configurationId, const uint32_t dataSizeInBytes) {
	const AllocatedBuffersSet::const_iterator it = ApproxInter::allocatedBuffers.find(BufferRange((uint8_t*) initialAddress, ((uint8_t*) initialAddress) + 1));

	if (it == ApproxInter::allocatedBuffers.cend()) {
		std::cout << "ApproxInter WARNING: buffer not marked in the first place." << std::endl;
	} else {
		ApproxInter::UnmarkBuffer(*it);

		ApproxInter::MarkBuffer(BufferRange(it->m_initialAddress, it->m_endAddress, bufferId, configurationId, dataSizeInBytes));
	}
}

void ApproxInter::MarkBuffer(const BufferRange& toMark) {
	const std::lock_guard<std::mutex> lock(ApproxInter::allocatedBuffersMutex);

	ApproxInter::allocatedBuffers.insert(toMark);
}

void ApproxInter::UnmarkBuffer(const BufferRange& toUnmark) {
	const std::lock_guard<std::mutex> lock(ApproxInter::allocatedBuffersMutex);

	/*const auto& it = ApproxInter::allocatedBuffers.find(toUnmark);
	if (it != ApproxInter::allocatedBuffers.cend()) {
		ApproxInter::unmarkedBuffers.insert(*it);
		//std::cout << "Unmarked Buffers: " << ApproxInter::unmarkedBuffers.size() << std::endl;
	}*/

	ApproxInter::allocatedBuffers.erase(toUnmark);
}

void ApproxInter::UnmarkBuffer(void const * const address) {
	ApproxInter::UnmarkBuffer(BufferRange((uint8_t*) address, ((uint8_t*) address) + 1));
}

void ApproxInter::UnmarkBuffer(void const * const start_address, void const * const endAddress) {
	ApproxInter::UnmarkBuffer(BufferRange((uint8_t*) start_address, (uint8_t*) endAddress));
}

void ApproxInter::InstrumentIfMarked(void * const address, const int64_t bufferId, const int64_t configurationId, const uint32_t dataSizeInBytes) {
	const BufferRange accessBuffer = BufferRange((uint8_t*) address, ((uint8_t*) address) + 1); //zero-sized access would be ignore in the case of a pointer to the buffer's first element

	const std::lock_guard<std::mutex> lock(ApproxInter::allocatedBuffersMutex);

	const AllocatedBuffersSet::const_iterator it = ApproxInter::allocatedBuffers.find(accessBuffer);

	if (it != ApproxInter::allocatedBuffers.cend()) {
		ApproxSS::add_approx(it->m_initialAddress, it->m_endAddress, bufferId, configurationId, dataSizeInBytes);
	} else {
		std::cout << "ApproxInter WARNING: buffer not marked for add_approx." << std::endl;
	}
}

void ApproxInter::InstrumentIfMarked(void * const address, const int64_t bufferIdPrefix, const int64_t configurationId) {
	const BufferRange accessBuffer = BufferRange((uint8_t*) address, ((uint8_t*) address) + 1); //zero-sized access would be ignore in the case of a pointer to the buffer's first element

	const std::lock_guard<std::mutex> lock(ApproxInter::allocatedBuffersMutex);

	const AllocatedBuffersSet::const_iterator it = ApproxInter::allocatedBuffers.find(accessBuffer);

	if (it != ApproxInter::allocatedBuffers.cend()) {
		ApproxSS::add_approx(it->m_initialAddress, it->m_endAddress, (bufferIdPrefix * ApproxInter::bufferIdPrefixFactor) + (bufferIdPrefix < 0 ? -it->m_bufferId : it->m_bufferId), configurationId, it->m_dataSizeInBytes);
	} else {
		std::cout << "ApproxInter WARNING: buffer not marked for add_approx." << std::endl;
	}
}

void ApproxInter::ReinstrumentIfMarked(void * const address, const int64_t bufferId, const int64_t configurationId, const uint32_t dataSizeInBytes) {
	ApproxInter::UninstrumentIfMarked(address, false);
	ApproxInter::InstrumentIfMarked(address, bufferId, configurationId, dataSizeInBytes);
}

void ApproxInter::UninstrumentIfMarked(void * const address, const bool giveAwayRecords /*= true*/) {
	const BufferRange accessBuffer = BufferRange((uint8_t*) address, ((uint8_t*) address) + 1); //zero-sized access would be ignore in the case of a pointer to the buffer's first element

	const std::lock_guard<std::mutex> lock(ApproxInter::allocatedBuffersMutex);

	const AllocatedBuffersSet::const_iterator it = ApproxInter::allocatedBuffers.find(accessBuffer);

	if (it != ApproxInter::allocatedBuffers.cend()) {
		ApproxSS::remove_approx(it->m_initialAddress, it->m_endAddress, giveAwayRecords);
	} else {
		std::cout << "ApproxInter WARNING: buffer not marked for remove_approx." << std::endl;
		/*std::cout << "ApproxInter WARNING: buffer not marked for remove_approx." << " Unmarked Buffers: " << ApproxInter::unmarkedBuffers.size() << "."; // << std::endl;
		const bool alreadyUnmarked = ApproxInter::unmarkedBuffers.find(accessBuffer) != ApproxInter::unmarkedBuffers.cend();
		if (alreadyUnmarked) {
			std::cout << " ALREADY UNMARKED."; 
		}
		std::cout << std::endl;*/
	}
}

#if COST_CAPTURE
void ApproxInter::ProcessTake(const uint8_t takeId, const uint8_t funcId, const uint64_t cost) {
	std::cout << Take::DFuncNames.at(funcId) << Take::Names[takeId] <<  cost << std::endl;
}
#endif

void ApproxInter::PrintMacroState(const std::string& macroName, const bool macroStatus, const std::string& tab /*= "\t"*/) {
	std::cout << tab << macroName << ": " << (macroStatus ? "Enabled" : "Disabled") << std::endl;
}

void ApproxInter::PrintMacrosStates() {
	std::cout << "APPROX MACROS STATES" << std::endl;

	ApproxInter::PrintMacroState("MATHEUS_INSTRUMENTATION", 			MATHEUS_INSTRUMENTATION);
	ApproxInter::PrintMacroState("FELIPE_INSTRUMENTATION", 				FELIPE_INSTRUMENTATION);

	ApproxInter::PrintMacroState("APPROX_RECO_BUFFER_INTER", 					APPROX_RECO_BUFFER_INTER);
	//ApproxInter::PrintMacroState("APPROX_RECO_BUFFER_INTER_MVP", 				APPROX_RECO_BUFFER_INTER_MVP);
	//ApproxInter::PrintMacroState("APPROX_RECO_BUFFER_INTER_PATTERN", 			APPROX_RECO_BUFFER_INTER_PATTERN);
	ApproxInter::PrintMacroState("APPROX_RECO_BUFFER_INTER_MVP_AND_PATTERN", 	APPROX_RECO_BUFFER_INTER_MVP_AND_PATTERN);
	ApproxInter::PrintMacroState("APPROX_RECO_BUFFER_INTER_TZ", 				APPROX_RECO_BUFFER_INTER_TZ);
	ApproxInter::PrintMacroState("APPROX_RECO_BUFFER_INTER_FAST", 				APPROX_RECO_BUFFER_INTER_FAST);
	ApproxInter::PrintMacroState("APPROX_RECO_BUFFER_INTER_REFINEMENT", 		APPROX_RECO_BUFFER_INTER_REFINEMENT);
	ApproxInter::PrintMacroState("APPROX_RECO_BUFFER_INTER_FRACTIONAL_WHOLE",	APPROX_RECO_BUFFER_INTER_FRACTIONAL_WHOLE);
	ApproxInter::PrintMacroState("APPROX_RECO_BUFFER_INTER_AFFINE", 			APPROX_RECO_BUFFER_INTER_AFFINE);

	ApproxInter::PrintMacroState("APPROX_ORIG_BUFFER_INTER_IME", 				APPROX_ORIG_BUFFER_INTER_IME);
	ApproxInter::PrintMacroState("APPROX_ORIG_BUFFER_INTER_REFINEMENT", 		APPROX_ORIG_BUFFER_INTER_REFINEMENT);
	ApproxInter::PrintMacroState("APPROX_ORIG_BUFFER_INTER_FRACTIONAL_WHOLE", 	APPROX_ORIG_BUFFER_INTER_FRACTIONAL_WHOLE);
	
	ApproxInter::PrintMacroState("APPROX_ORIG_BUFFER_INTER", 					APPROX_ORIG_BUFFER_INTER);
	ApproxInter::PrintMacroState("APPROX_ORIG_BUFFER_INTER_AFFINE", 			APPROX_ORIG_BUFFER_INTER_AFFINE);
	ApproxInter::PrintMacroState("APPROX_FILT_BUFFER_V1", 						APPROX_FILT_BUFFER_V1);
	ApproxInter::PrintMacroState("APPROX_FILT_BUFFER_V2", 						APPROX_FILT_BUFFER_V2);
	ApproxInter::PrintMacroState("APPROX_PRED_BUFFER", 							APPROX_PRED_BUFFER);

	ApproxInter::PrintMacroState("APPROX_FME_BEST_MV_COST_RECALC", 				APPROX_FME_BEST_MV_COST_RECALC);

	ApproxInter::PrintMacroState("COST_CAPTURE", 							COST_CAPTURE);

	ApproxInter::PrintMacroState("MATHEUS_SKIP_FRACTIONAL_MOTION_ESTIMATION", MATHEUS_SKIP_FRACTIONAL_MOTION_ESTIMATION);
	std::cout << "\tMATHEUS_xPatternSearchIntRefine_ITERATED_POS" << ": " << MATHEUS_xPatternSearchIntRefine_ITERATED_POS << std::endl;

	ApproxInter::PrintMacroState("PRINT_COST", 							PRINT_COST);

	std::cout << std::endl;
}

void ApproxInter::PrintBufferInfo(const std::string& bufferName, const int64_t bufferId, const int64_t configurationId, const std::string& tab/* = "\t"*/) {
	std::cout << tab << bufferName << " = BufferId: " << bufferId << ", ConfigurationId: " << configurationId << std::endl;
}

void ApproxInter::PrintBuffersInfo() {
	std::cout << "APPROX BUFFERS INFO" << std::endl;

	/*ApproxInter::PrintBufferInfo("RECO_MOTION_ESTIMATION", 					ApproxInter::BufferId::RECO_MOTION_ESTIMATION, 					ApproxInter::ConfigurationId::RECO_MOTION_ESTIMATION);
	ApproxInter::PrintBufferInfo("RECO_AFFINE_MOTION_ESTIMATION", 			ApproxInter::BufferId::RECO_AFFINE_MOTION_ESTIMATION, 			ApproxInter::ConfigurationId::RECO_AFFINE_MOTION_ESTIMATION);
	ApproxInter::PrintBufferInfo("ORIG_MOTION_ESTIMATION", 					ApproxInter::BufferId::ORIG_MOTION_ESTIMATION,					ApproxInter::ConfigurationId::ORIG_MOTION_ESTIMATION);

	ApproxInter::PrintBufferInfo("TEMP_ORIG_MOTION_ESTIMATION", 			ApproxInter::BufferId::TEMP_ORIG_MOTION_ESTIMATION, 			ApproxInter::ConfigurationId::TEMP_ORIG_MOTION_ESTIMATION);
	ApproxInter::PrintBufferInfo("ORIG_AFFINE_MOTION_ESTIMATION", 			ApproxInter::BufferId::ORIG_AFFINE_MOTION_ESTIMATION, 			ApproxInter::ConfigurationId::ORIG_AFFINE_MOTION_ESTIMATION);
	ApproxInter::PrintBufferInfo("TEMP_ORIG_AFFINE_MOTION_ESTIMATION",		ApproxInter::BufferId::TEMP_ORIG_AFFINE_MOTION_ESTIMATION,		ApproxInter::ConfigurationId::TEMP_ORIG_AFFINE_MOTION_ESTIMATION);

	//ApproxInter::PrintBufferInfo("FILT_MOTION_ESTIMATION_TEMP", 			ApproxInter::BufferId::FILT_MOTION_ESTIMATION_TEMP, 			ApproxInter::ConfigurationId::FILT_MOTION_ESTIMATION_TEMP);
	//ApproxInter::PrintBufferInfo("FILT_MOTION_ESTIMATION", 					ApproxInter::BufferId::FILT_MOTION_ESTIMATION, 					ApproxInter::ConfigurationId::FILT_MOTION_ESTIMATION);
	ApproxInter::PrintBufferInfo("PRED_AFFINE_MOTION_ESTIMATION", 			ApproxInter::BufferId::PRED_AFFINE_MOTION_ESTIMATION,			ApproxInter::ConfigurationId::PRED_AFFINE_MOTION_ESTIMATION);

	//ApproxInter::PrintBufferInfo("RECO_MOTION_ESTIMATION_MVP", 			ApproxInter::BufferId::RECO_MOTION_ESTIMATION_MVP, 				ApproxInter::ConfigurationId::RECO_MOTION_ESTIMATION_MVP);
	//ApproxInter::PrintBufferInfo("RECO_MOTION_ESTIMATION_PATTERN", 		ApproxInter::BufferId::RECO_MOTION_ESTIMATION_PATTERN, 			ApproxInter::ConfigurationId::RECO_MOTION_ESTIMATION_PATTERN);
	ApproxInter::PrintBufferInfo("RECO_MOTION_ESTIMATION_MVP_AND_PATTERN", 	ApproxInter::BufferId::RECO_MOTION_ESTIMATION_MVP_AND_PATTERN, 	ApproxInter::ConfigurationId::RECO_MOTION_ESTIMATION_MVP_AND_PATTERN);
	ApproxInter::PrintBufferInfo("RECO_MOTION_ESTIMATION_TZ", 				ApproxInter::BufferId::RECO_MOTION_ESTIMATION_TZ,				ApproxInter::ConfigurationId::RECO_MOTION_ESTIMATION_TZ);

	ApproxInter::PrintBufferInfo("RECO_MOTION_ESTIMATION_FAST", 			ApproxInter::BufferId::RECO_MOTION_ESTIMATION_FAST, 			ApproxInter::ConfigurationId::RECO_MOTION_ESTIMATION_FAST);
	ApproxInter::PrintBufferInfo("RECO_MOTION_ESTIMATION_FRACTIONAL", 		ApproxInter::BufferId::RECO_MOTION_ESTIMATION_FRACTIONAL, 		ApproxInter::ConfigurationId::RECO_MOTION_ESTIMATION_FRACTIONAL);
	ApproxInter::PrintBufferInfo("RECO_MOTION_ESTIMATION_REFINEMENT", 		ApproxInter::BufferId::RECO_MOTION_ESTIMATION_REFINEMENT,		ApproxInter::ConfigurationId::RECO_MOTION_ESTIMATION_REFINEMENT);

	ApproxInter::PrintBufferInfo("ORIG_MOTION_ESTIMATION_IME", 				ApproxInter::BufferId::ORIG_MOTION_ESTIMATION_IME, 				ApproxInter::ConfigurationId::ORIG_MOTION_ESTIMATION_IME);
	ApproxInter::PrintBufferInfo("ORIG_MOTION_ESTIMATION_FRACTIONAL", 		ApproxInter::BufferId::ORIG_MOTION_ESTIMATION_FRACTIONAL, 		ApproxInter::ConfigurationId::ORIG_MOTION_ESTIMATION_FRACTIONAL);
	ApproxInter::PrintBufferInfo("ORIG_MOTION_ESTIMATION_REFINEMENT", 		ApproxInter::BufferId::ORIG_MOTION_ESTIMATION_REFINEMENT,		ApproxInter::ConfigurationId::ORIG_MOTION_ESTIMATION_REFINEMENT);*/


	//tinha jeito mais elegante de fazer isso
	/////////////////////////////////////////////////////////////////////////////////////////////////////
	/*std::cout << "\tFME_FILT = BufferId: {";
	for (auto i = 0; i < vvenc::LUMA_INTERPOLATION_FILTER_SUB_SAMPLE_POSITIONS_SIGNAL; ++i) {
		std::cout << "{";
		const std::string i_string = ApproxInter::BufferId::FME_FILT_OFFSET + std::to_string(i);

		for (auto j = 0; j < vvenc::LUMA_INTERPOLATION_FILTER_SUB_SAMPLE_POSITIONS_SIGNAL; ++j) {
			std::cout << "{";
			const std::string j_string = std::to_string(j);
			const std::string ij_string = i_string + j_string;

			for (auto k = 0; k < vvenc::ComponentID::MAX_NUM_COMP; ++k) {
				const std::string k_string = std::to_string(k);
				const std::string ijk_string = ij_string + k_string;

				const int64_t ijk = std::stol(ijk_string);
				ApproxInter::BufferId::FME_FILT[i][j][k] = ijk;

				std::cout << ijk_string;
				if (k + 1 != vvenc::ComponentID::MAX_NUM_COMP) {std::cout << ',';}
			}

			std::cout << "}";
			if (j + 1 != vvenc::LUMA_INTERPOLATION_FILTER_SUB_SAMPLE_POSITIONS_SIGNAL) {std::cout << ',';}
		}

		std::cout << "}";
		if (i + 1 != vvenc::LUMA_INTERPOLATION_FILTER_SUB_SAMPLE_POSITIONS_SIGNAL) {std::cout << ',';}
	}
	std::cout << "}, ConfigurationId: " << ApproxInter::ConfigurationId::FME_FILT << std::endl;*/


	//////////////////////////////////////////////////////////////////////////////////////////////////////	
	/*std::cout << "\tFME_FILT_TEMP = BufferId: {";
	for (auto j = 0; j < vvenc::LUMA_INTERPOLATION_FILTER_SUB_SAMPLE_POSITIONS_SIGNAL; ++j) {
		std::cout << "{";
		const std::string j_string = ApproxInter::BufferId::FME_FILT_TEMP_OFFSET + std::to_string(j);

		for (auto k = 0; k < vvenc::ComponentID::MAX_NUM_COMP; ++k) {
			const std::string k_string = std::to_string(k);
			const std::string jk_string = j_string + k_string;

			const int64_t ijk = std::stol(jk_string);
			ApproxInter::BufferId::FME_FILT_TEMP[j][k] = ijk;

			std::cout << jk_string;
			if (k + 1 != vvenc::ComponentID::MAX_NUM_COMP) {std::cout << ',';}
		}

		std::cout << "}";
		if (j + 1 != vvenc::LUMA_INTERPOLATION_FILTER_SUB_SAMPLE_POSITIONS_SIGNAL) {std::cout << ',';}
	}
	std::cout << "}, ConfigurationId: " << ApproxInter::ConfigurationId::FME_FILT_TEMP << std::endl;*/
	/////////////////////////////////////////////////////////////////////////////////////////////////////


	std::cout << std::endl;
}

#if FELIPE_INSTRUMENTATION
	#if APPROX_RECO_BUFFER_INTER
		//double ApproxInter::MEReadBER;
		//double ApproxInter::MEWriteBER;

		unsigned ApproxInter::RECO::frameBufferWidth, ApproxInter::RECO::frameBufferHeight;
		unsigned ApproxInter::RECO::xMargin, ApproxInter::RECO::yMargin;

		bool ApproxInter::RECO::collectBufferSize, ApproxInter::RECO::tmpBool;

		int ApproxInter::RECO::debugEnable;

		std::fstream ApproxInter::RECO::fp;

		void ApproxInter::RECO::init() {
			//MEReadBER = 0.0;
			//MEWriteBER = 0.0;
			collectBufferSize = true;
			tmpBool = false;
		}

		void ApproxInter::RECO::initDebug() {
			if(debugEnable) {
				fp.open("debug.txt", std::fstream::out);
			}
		}
	#endif

	#if APPROX_ORIG_BUFFER_INTER
		//double ApproxInter::MEReadBER;
		//double ApproxInter::MEWriteBER;

		unsigned ApproxInter::ORIG::frameOrigBufferWidth, ApproxInter::ORIG::frameOrigBufferHeight;
		//unsigned ApproxInter::xMargin, ApproxInter::yMargin;

		bool ApproxInter::ORIG::collectBufferSize, ApproxInter::ORIG::tmpBool;

		int ApproxInter::ORIG::debugEnable;

		std::fstream ApproxInter::ORIG::fp;

		void ApproxInter::ORIG::init() {
			collectBufferSize = true;
			tmpBool = false;
		}

		void ApproxInter::ORIG::initDebug() {
			if(debugEnable) {
				fp.open("debug.txt", std::fstream::out);
			}
		}
	#endif

	#if APPROX_FILT_BUFFER_V1 || APPROX_FILT_BUFFER_V2	
		//<Felipe>
			int ApproxInter::FILT::extWidthFiltered, ApproxInter::FILT::extHeightFiltered;
		//</Felipe>
	#endif
#endif