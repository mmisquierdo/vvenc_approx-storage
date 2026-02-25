#ifndef APPROXME_H
#define  APPROXME_H

  #include <iostream>
  #include <fstream>
  #include <set>
  #include <mutex>
  #include <array>
  #include <unordered_map>
  #include <cstdint>
  #include <functional>
  #include "approx.h"

  #define MATHEUS_INSTRUMENTATION       false
  #define FELIPE_INSTRUMENTATION (!MATHEUS_INSTRUMENTATION && false)

  #define MATHEUS_SKIP_FRACTIONAL_MOTION_ESTIMATION false
  #define MATHEUS_xPatternSearchIntRefine_ITERATED_POS 9 /*up to 9: default*/

  #define APPROX_RECO_BUFFER_INTER                    false   //ativa instrumentação em toda a ME (excluindo affine)
  #define APPROX_ORIG_BUFFER_INTER                    false   //ativa instrumentação em toda a ME (excluindo affine), para ORIG e TEMP_ORIG

  //#define APPROX_RECO_BUFFER_INTER_MVP     true 
  //#define APPROX_RECO_BUFFER_INTER_PATTERN   true
  #define APPROX_RECO_BUFFER_INTER_MVP_AND_PATTERN    false
  #define APPROX_RECO_BUFFER_INTER_TZ                 false
  #define APPROX_RECO_BUFFER_INTER_FAST               false
  #define APPROX_RECO_BUFFER_INTER_REFINEMENT         false
  #define APPROX_RECO_BUFFER_INTER_FRACTIONAL_WHOLE   false

  #define APPROX_ORIG_BUFFER_INTER_IME                false
  #define APPROX_ORIG_BUFFER_INTER_REFINEMENT         false
  #define APPROX_ORIG_BUFFER_INTER_FRACTIONAL_WHOLE   false

  #define APPROX_RECO_BUFFER_INTER_AFFINE             false
  #define APPROX_ORIG_BUFFER_INTER_AFFINE             false  //TODO!!!: check for temporary copies

  #define APPROX_FILT_BUFFER_V1                       false  //GENERALIST
  #define APPROX_FILT_BUFFER_V2                       false  //SPECIFIC
  #define APPROX_PRED_BUFFER                          false


  #define APPROX_FME_BEST_MV_COST_RECALC              false
  #define APPROX_FME_RECO                             false
  #define APPROX_FME_ORIG                             false
  #define APPROX_FME_FILT                             false

  #define COST_CAPTURE                                true //i'm iffy if it's corretly implemented on ARM (RdCostARM.h), actually new CostCapture should do it 
  #define LONG_DOUBLELOG                              false
  #define COST_DOUBLELOG                              true

  #define INSTRUMENT_METRICS                          false

  #define CAPTURED_METRIC_INSTRUMENTATION             true

  /*#define APPROX_FME_HP_RECO              true
  #define APPROX_FME_HP_ORIG              true
  #define APPROX_FME_QP_RECO              true
  #define APPROX_FME_QP_ORIG              true*/

  #define PRINT_COST                                  false

  class BufferRange {
    public:
      uint8_t * const m_initialAddress;
      uint8_t const * const m_endAddress;
      const int64_t m_bufferId;
      const int64_t m_configurationId;
      const uint32_t m_dataSizeInBytes;

      BufferRange(uint8_t * const initialAddress, uint8_t const * const endAddress);

      BufferRange(uint8_t * const initialAddress, uint8_t const * const endAddress, const int64_t bufferId, const int64_t configurationId, const uint32_t dataSizeInBytes);

      //overlapping ranges are considered equivalent
      friend bool operator<(const BufferRange& lhv, const BufferRange& rhv) {  
        return lhv.m_endAddress <= rhv.m_initialAddress;
      }
  };

  typedef std::set<BufferRange> AllocatedBuffersSet;

  namespace ApproxInter {
      constexpr int64_t bufferIdPrefixFactor = 1000000; 
    //private:
      extern AllocatedBuffersSet allocatedBuffers; //use methods to manipulate
      //extern AllocatedBuffersSet unmarkedBuffers;
      extern std::mutex allocatedBuffersMutex;

      //FME_BEST_MV_COST_RECALC
      extern uint32_t fme_uiDirecBest;

      extern int lastFuncId;

    //public:
      #if COST_CAPTURE
      namespace Take {
        constexpr uint64_t Approximate = 0;
        constexpr uint64_t Precise = 1;
        constexpr uint64_t Size = 2;

        constexpr std::array<char const*const, Size> Names = {"~", "="};

        extern uint64_t CurrentTake;

        enum DFunc : uint8_t //gambiarra feia pra nao dar um include quebrado
        {
        DF_SSE             = 0,             ///< general size SSE
        DF_SSE2            = DF_SSE+1,      ///<   2xM SSE
        DF_SSE4            = DF_SSE+2,      ///<   4xM SSE
        DF_SSE8            = DF_SSE+3,      ///<   8xM SSE
        DF_SSE16           = DF_SSE+4,      ///<  16xM SSE
        DF_SSE32           = DF_SSE+5,      ///<  32xM SSE
        DF_SSE64           = DF_SSE+6,      ///<  64xM SSE
        DF_SSE128          = DF_SSE+7,      ///< 16NxM SSE

        DF_SAD             = 8,             ///< general size SAD
        DF_SAD2            = DF_SAD+1,      ///<   2xM SAD
        DF_SAD4            = DF_SAD+2,      ///<   4xM SAD
        DF_SAD8            = DF_SAD+3,      ///<   8xM SAD
        DF_SAD16           = DF_SAD+4,      ///<  16xM SAD
        DF_SAD32           = DF_SAD+5,      ///<  32xM SAD
        DF_SAD64           = DF_SAD+6,      ///<  64xM SAD
        DF_SAD128          = DF_SAD+7,      ///< 16NxM SAD

        DF_HAD             = 16,            ///< general size Hadamard
        DF_HAD2            = DF_HAD+1,      ///<   2xM HAD
        DF_HAD4            = DF_HAD+2,      ///<   4xM HAD
        DF_HAD8            = DF_HAD+3,      ///<   8xM HAD
        DF_HAD16           = DF_HAD+4,      ///<  16xM HAD
        DF_HAD32           = DF_HAD+5,      ///<  32xM HAD
        DF_HAD64           = DF_HAD+6,      ///<  64xM HAD
        DF_HAD128          = DF_HAD+7,      ///< 16NxM HAD

        DF_SAD_MASKED      = 24,      
        DF_SAD_MASKED2     = DF_SAD_MASKED+1,
        DF_SAD_MASKED4     = DF_SAD_MASKED+2,
        DF_SAD_MASKED8     = DF_SAD_MASKED+3,
        DF_SAD_MASKED16    = DF_SAD_MASKED+4,
        DF_SAD_MASKED32    = DF_SAD_MASKED+5,
        DF_SAD_MASKED64    = DF_SAD_MASKED+6,
        DF_SAD_MASKED128   = DF_SAD_MASKED+7,        
        
        DF_HAD_fast        = 32,                 ///< general size Hadamard
        DF_HAD2_fast       = DF_HAD_fast+1,      ///<   2xM fast HAD
        DF_HAD4_fast       = DF_HAD_fast+2,      ///<   4xM fast HAD
        DF_HAD8_fast       = DF_HAD_fast+3,      ///<   8xM fast HAD
        DF_HAD16_fast      = DF_HAD_fast+4,      ///<  16xM fast HAD
        DF_HAD32_fast      = DF_HAD_fast+5,      ///<  32xM fast HAD
        DF_HAD64_fast      = DF_HAD_fast+6,      ///<  64xM fast HAD
        DF_HAD128_fast     = DF_HAD_fast+7,      ///< 16NxM fast HAD

        DF_HAD_2SAD        = 40,                 //tbd th remove

        DF_TOTAL_FUNCTIONS = 41,

        DF_SSE_WTD         = 42, //0xf2u         // out of func scope
        DF_SSE_WTD2        = DF_SSE_WTD+1,
        DF_SSE_WTD4        = DF_SSE_WTD+2,
        DF_SSE_WTD8        = DF_SSE_WTD+3,
        DF_SSE_WTD16       = DF_SSE_WTD+4,
        DF_SSE_WTD32       = DF_SSE_WTD+5,
        DF_SSE_WTD64       = DF_SSE_WTD+6,
        DF_SSE_WTD128      = DF_SSE_WTD+7,

        DF_SAD8XN          = 50,
        DF_SAD8XN2         = DF_SAD8XN+1,
        DF_SAD8XN4         = DF_SAD8XN+2,
        DF_SAD8XN8         = DF_SAD8XN+3,
        DF_SAD8XN16        = DF_SAD8XN+4,
        DF_SAD8XN32        = DF_SAD8XN+5,
        DF_SAD8XN64        = DF_SAD8XN+6,
        DF_SAD8XN128       = DF_SAD8XN+7,
        DF_SAD16XN         = 58,
        DF_SAD16XN2        = DF_SAD16XN+1,
        DF_SAD16XN4        = DF_SAD16XN+2,
        DF_SAD16XN8        = DF_SAD16XN+3,
        DF_SAD16XN16       = DF_SAD16XN+4,
        DF_SAD16XN32       = DF_SAD16XN+5,
        DF_SAD16XN64       = DF_SAD16XN+6,
        DF_SAD16XN128      = DF_SAD16XN+7,

        DF_TOTAL_FUNCTIONS_ACTUAL = 66,
        DF_OUT_OF_RANGE    = 67
        };

        const std::unordered_map<const uint8_t, char const * const, std::hash<uint8_t>> DFuncNames = {
          { DF_SSE,             "SSE" },
          { DF_SSE2,            "SSE2" },
          { DF_SSE4,            "SSE4" },
          { DF_SSE8,            "SSE8" },
          { DF_SSE16,           "SSE16" },
          { DF_SSE32,           "SSE32" },
          { DF_SSE64,           "SSE64" },
          { DF_SSE128,          "SSE128" },

          { DF_SAD,             "SAD" },
          { DF_SAD2,            "SAD2" },
          { DF_SAD4,            "SAD4" },
          { DF_SAD8,            "SAD8" },
          { DF_SAD16,           "SAD16" },
          { DF_SAD32,           "SAD32" },
          { DF_SAD64,           "SAD64" },
          { DF_SAD128,          "SAD128" },

          { DF_HAD,             "HAD" },
          { DF_HAD2,            "HAD2" },
          { DF_HAD4,            "HAD4" },
          { DF_HAD8,            "HAD8" },
          { DF_HAD16,           "HAD16" },
          { DF_HAD32,           "HAD32" },
          { DF_HAD64,           "HAD64" },
          { DF_HAD128,          "HAD128" },

          { DF_SAD_MASKED,      "SAD_MASKED" },
          { DF_SAD_MASKED2,     "SAD_MASKED2" },
          { DF_SAD_MASKED4,     "SAD_MASKED4" },
          { DF_SAD_MASKED8,     "SAD_MASKED8" },
          { DF_SAD_MASKED16,    "SAD_MASKED16" },
          { DF_SAD_MASKED32,    "SAD_MASKED32" },
          { DF_SAD_MASKED64,    "SAD_MASKED64" },
          { DF_SAD_MASKED128,   "SAD_MASKED128" },

          { DF_HAD_fast,        "HAD_fast" },
          { DF_HAD2_fast,       "HAD2_fast" },
          { DF_HAD4_fast,       "HAD4_fast" },
          { DF_HAD8_fast,       "HAD8_fast" },
          { DF_HAD16_fast,      "HAD16_fast" },
          { DF_HAD32_fast,      "HAD32_fast" },
          { DF_HAD64_fast,      "HAD64_fast" },
          { DF_HAD128_fast,     "HAD128_fast" },

          { DF_HAD_2SAD,        "HAD_2SAD" },

          { DF_TOTAL_FUNCTIONS, "TOTAL_FUNCTIONS" },
          { DF_SSE_WTD,         "SSE_WTD" },
          { DF_SSE_WTD2,        "SSE_WTD2" },
          { DF_SSE_WTD4,        "SSE_WTD4" },
          { DF_SSE_WTD8,        "SSE_WTD8" },
          { DF_SSE_WTD16,       "SSE_WTD16" },
          { DF_SSE_WTD32,       "SSE_WTD32" },
          { DF_SSE_WTD64,       "SSE_WTD64" },
          { DF_SSE_WTD128,      "SSE_WTD128" },

          {DF_SAD8XN,           "SAD8XN"},
          {DF_SAD8XN2,          "SAD8XN2"},
          {DF_SAD8XN4,          "SAD8XN4"},
          {DF_SAD8XN8,          "SAD8XN8"},
          {DF_SAD8XN16,         "SAD8XN16"},
          {DF_SAD8XN32,         "SAD8XN32"},
          {DF_SAD8XN64,         "SAD8XN64"},
          {DF_SAD8XN128,        "SAD8XN128"},
          {DF_SAD16XN,          "SAD16XN"},
          {DF_SAD16XN2,         "SAD16XN2"},
          {DF_SAD16XN4,         "SAD16XN4"},
          {DF_SAD16XN8,         "SAD16XN8"},
          {DF_SAD16XN16,        "SAD16XN16"},
          {DF_SAD16XN32,        "SAD16XN32"},
          {DF_SAD16XN64,        "SAD16XN64"},
          {DF_SAD16XN128,       "SAD16XN128"},

          { DF_OUT_OF_RANGE,    "OUT_OF_RANGE"}
        };
      }
      #endif

    
      namespace BufferId {
        constexpr int64_t OTHERS = 0;
        constexpr int64_t PIC_RECONSTRUCTION_Y = 10;

        constexpr int64_t CS_PIC_RECONSTRUCTION_Y = 910;

        constexpr int64_t PIC_ORIGINAL_Y = 20;

        constexpr int64_t PIC_RESIDUAL_Y = 50;

        constexpr int64_t CS_PIC_RESIDUAL_Y = 950;

        constexpr int64_t PIC_PREDICTION_Y = 40;

        constexpr int64_t CS_PIC_PREDICTION_Y = 940;

        constexpr int64_t PIC_SAO_TEMP_Y = 70;

        constexpr int64_t PIC_ORIGINAL_RSP_REC_Y = 90;

        constexpr int64_t CS_PIC_ORIGINAL_RSP_REC_Y = 990;

        constexpr int64_t AdaptiveLoopFilter_tempBuf = 100;

        constexpr int64_t AdaptiveLoopFilter_tempBuf2 = 110;

        constexpr int64_t mipMatrixMulCore_buffer = 120;

        constexpr int64_t CompStorage_mMemory = 130;

        constexpr int64_t filterC = 140;

        constexpr int64_t scalarFilterN2_2D_tmp = 150; 

        constexpr int64_t filterXxY_N2_cH = 160; 
        constexpr int64_t filterXxY_N2_cV = 170; 
        constexpr int64_t filterXxY_N2_temp_int = 180; 

        constexpr int64_t filterXxY_N4_cH = 190; 
        constexpr int64_t filterXxY_N4_cV = 200; 
        constexpr int64_t filterXxY_N4_temp_int = 210; 

        constexpr int64_t filterXxY_N8_cH = 220; 
        constexpr int64_t filterXxY_N8_cV = 230; 
        constexpr int64_t filterXxY_N8_temp_int = 240; 

        constexpr int64_t InterPrediction_m_yuvPred = 250;
        
        constexpr int64_t InterPrediction_m_geoPartBuf_0 = 260;

        constexpr int64_t InterPrediction_m_geoPartBuf_1 = 270;

        constexpr int64_t InterPrediction_m_IBCBuffer = 280;

        constexpr int64_t InterPredInterpolation_m_filteredBlockTmp = 290;

        constexpr int64_t InterPredInterpolation_m_filteredBlock = 300;

        constexpr int64_t InterPredInterpolation_m_gradX0 = 310;
        constexpr int64_t InterPredInterpolation_m_gradY0 = 320;
        constexpr int64_t InterPredInterpolation_m_gradX1 = 330;
        constexpr int64_t InterPredInterpolation_m_gradY1 = 340;

        constexpr int64_t DMVR_m_yuvPred = 350;
        constexpr int64_t DMVR_m_yuvTmp = 360;
        constexpr int64_t DMVR_m_yuvPad = 370;

        constexpr int64_t IntraPredAngleLuma_Core_p = 380;

        constexpr int64_t IntraPrediction_m_pMdlmTemp = 390;

        constexpr int64_t IntraPrediction_xPredIntraAng_refAbove = 400;
        constexpr int64_t IntraPrediction_xPredIntraAng_refLeft = 410;

        constexpr int64_t IntraPrediction_xPredIntraAng_tempArray = 420;
        constexpr int64_t IntraPrediction_xPredIntraAng_p = 430;


        constexpr int64_t IntraPrediction_xGetLMParameters_selectLumaPix = 440;
        constexpr int64_t IntraPrediction_xGetLMParameters_selectChromaPix = 450;

        constexpr int64_t MatrixIntraPrediction_m_reducedBoundary = 460;
        constexpr int64_t MatrixIntraPrediction_m_reducedBoundaryTransp = 470;

        constexpr int64_t MatrixIntraPrediction_predBlock_bufReducedPred = 480;


        constexpr int64_t MCTF_motionErrorLumaFrac6_tempArray = 490;
        constexpr int64_t MCTF_motionErrorLumaFrac4_tempArray = 500;
        constexpr int64_t MCTF_applyFrac8Core_6Tap_tempArray = 510;
        constexpr int64_t MCTF_applyFrac8Core_4Tap_tempArray = 520;
        constexpr int64_t MCTF_filter_fltrBuf1 = 530;
        constexpr int64_t MCTF_filter_fltrBuf2 = 540;
        constexpr int64_t MCTF_subsampleLuma_output = 550;
        constexpr int64_t MCTF_xFinalizeBlkLine_dstBufs = 560;

        constexpr int64_t InterpolationFilterX86_simdFilter_c = 570;

        constexpr int64_t InterpolationFilterX86_simdFilter16xX_N8_tmp = 580;
        constexpr int64_t InterpolationFilterX86_simdFilter16xX_N4_tmp = 590;
        constexpr int64_t InterpolationFilterX86_simdFilter8xX_N8_tmp = 600;
        constexpr int64_t InterpolationFilterX86_simdFilter8xX_N4_tmp = 610;

        constexpr int64_t DecCu_m_TmpBuffer = 620;
        constexpr int64_t DecCu_m_PredBuffer = 630;

        constexpr int64_t EncAdaptiveLoopFilter_m_bufOrigin = 640;

        constexpr int64_t EncAdaptiveLoopFilter_getPredBlkStats_yLocal = 650;
        constexpr int64_t EncAdaptiveLoopFilter_getPredBlkStats_ELocal = 660;

        constexpr int64_t EncAdaptiveLoopFilter_getBlkStatsCcAlf_yLocal = 670;
        constexpr int64_t EncAdaptiveLoopFilter_getBlkStatsCcAlf_ELocal = 680;

        constexpr int64_t EncCu_m_pOrgBuffer = 690;
        constexpr int64_t EncCu_m_pRspBuffer = 700;
        constexpr int64_t EncCu_m_aTmpStorageLCU = 710;
        constexpr int64_t EncCu_m_acMergeTmpBuffer = 720; 
        constexpr int64_t EncCu_m_dbBuffer = 730; 

        constexpr int64_t EncGOP_xInitLMCS_getFilteredOrigBuffer = 740;

        constexpr int64_t EncSlice_m_alfTempCtuBuf = 750;

        constexpr int64_t InterSearch_m_tmpPredStorage = 760;
        constexpr int64_t InterSearch_m_tmpStorageLCU = 770;
        constexpr int64_t InterSearch_m_pTempPel = 780;
        constexpr int64_t InterSearch_m_tmpAffiStorage = 790;
        constexpr int64_t InterSearch_m_tmpAffiError = 800; 
        constexpr int64_t InterSearch_m_tmpAffiDeri0 = 810; 
        constexpr int64_t InterSearch_m_tmpAffiDeri1 = 820;  
        constexpr int64_t vvenc_YUVBuffer_alloc_buffer_yuvPlane_int16 = 830;

        constexpr int64_t AdaptiveLoopFilter_m_alfClippingValues = 840;

        constexpr int64_t InterPredInterpolation_m_gradBuf = 850;

        constexpr int64_t InterInterpolation_m_refBuffer = 860;

        constexpr int64_t EncReshape_m_cwLumaWeight = 870;

        constexpr int64_t MergeItem_m_pelStorage = 880;

        constexpr int64_t Canny_m_orientationBuf = 1000;
        constexpr int64_t Canny_m_gradientBufX = 1010;
        constexpr int64_t Canny_m_gradientBufY = 1020;

        constexpr int64_t Morph_m_dilationBuf = 1030;
        constexpr int64_t Morph_m_dilationBuf2 = 1040;
        constexpr int64_t Morph_m_dilationBuf4 = 1050;

        constexpr int64_t FGAnalyzer_m_maskBuf = 1060;
        constexpr int64_t FGAnalyzer_m_grainEstimateBuf = 1070;
        constexpr int64_t FGAnalyzer_m_workingBufSubsampled2 = 1080;
        constexpr int64_t FGAnalyzer_m_maskSubsampled2 = 1100;
        constexpr int64_t FGAnalyzer_m_workingBufSubsampled4 = 1110;
        constexpr int64_t FGAnalyzer_m_maskSubsampled4 = 1120;
        constexpr int64_t FGAnalyzer_m_maskUpsampled = 1130;

        constexpr int64_t SortedPelUnitBufs_m_acStorage = 1140;


        //PREFIXES
        constexpr int64_t SAD_Orig =  11;
        constexpr int64_t SAD_Curr = -11;

        constexpr int64_t MaskedSAD_Orig =  12;
        constexpr int64_t MaskedSAD_Curr = -12;

        constexpr int64_t SAD8XN_Orig =  13;
        constexpr int64_t SAD8XN_Curr = -13;

        constexpr int64_t SAD16XN_Orig =  14;
        constexpr int64_t SAD16XN_Curr = -14;

        constexpr int64_t SSE_Orig =  15;
        constexpr int64_t SSE_Curr = -15;

        constexpr int64_t WeightedSSE_Orig =  16;
        constexpr int64_t WeightedSSE_Curr = -16;

        constexpr int64_t HAD_Orig =  17;
        constexpr int64_t HAD_Curr = -17;

        constexpr int64_t FastHAD_Orig =  18;
        constexpr int64_t FastHAD_Curr = -18;

        constexpr int64_t offsetBufferId(const int64_t base, const int64_t suffix, const int64_t factor = bufferIdPrefixFactor) {
          return base * factor + (base < 0 ? -suffix : suffix);
        }

        constexpr std::array<const int64_t, Take::DF_TOTAL_FUNCTIONS_ACTUAL> DFunc_Orig = {
          offsetBufferId(SSE_Orig, 1),      //{ Take::DF_SSE,             "SSE" },
          offsetBufferId(SSE_Orig, 2),      //{ Take::DF_SSE2,            "SSE2" },
          offsetBufferId(SSE_Orig, 4),      //{ Take::DF_SSE4,            "SSE4" },
          offsetBufferId(SSE_Orig, 8),      //{ Take::DF_SSE8,            "SSE8" },
          offsetBufferId(SSE_Orig, 16),     //{ Take::DF_SSE16,           "SSE16" },
          offsetBufferId(SSE_Orig, 32),     //{ Take::DF_SSE32,           "SSE32" },
          offsetBufferId(SSE_Orig, 64),     //{ Take::DF_SSE64,           "SSE64" },
          offsetBufferId(SSE_Orig, 128),    //{ Take::DF_SSE128,          "SSE128" },

          offsetBufferId(SAD_Orig, 1),      //{ Take::DF_SAD,             "SAD" },
          offsetBufferId(SAD_Orig, 2),      //{ Take::DF_SAD2,            "SAD2" },
          offsetBufferId(SAD_Orig, 4),      //{ Take::DF_SAD4,            "SAD4" },
          offsetBufferId(SAD_Orig, 8),      //{ Take::DF_SAD8,            "SAD8" },
          offsetBufferId(SAD_Orig, 16),     //{ Take::DF_SAD16,           "SAD16" },
          offsetBufferId(SAD_Orig, 32),     //{ Take::DF_SAD32,           "SAD32" },
          offsetBufferId(SAD_Orig, 64),     //{ Take::DF_SAD64,           "SAD64" },
          offsetBufferId(SAD_Orig, 128),    //{ Take::DF_SAD128,          "SAD128" },

          offsetBufferId(HAD_Orig, 1),      //{ Take::DF_HAD,             "HAD" },
          offsetBufferId(HAD_Orig, 2),      //{ Take::DF_HAD2,            "HAD2" },
          offsetBufferId(HAD_Orig, 4),      //{ Take::DF_HAD4,            "HAD4" },
          offsetBufferId(HAD_Orig, 8),      //{ Take::DF_HAD8,            "HAD8" },
          offsetBufferId(HAD_Orig, 16),     //{ Take::DF_HAD16,           "HAD16" },
          offsetBufferId(HAD_Orig, 32),     //{ Take::DF_HAD32,           "HAD32" },
          offsetBufferId(HAD_Orig, 64),     //{ Take::DF_HAD64,           "HAD64" },
          offsetBufferId(HAD_Orig, 128),    //{ Take::DF_HAD128,          "HAD128" },

          offsetBufferId(MaskedSAD_Orig, 1),    //{ Take::DF_SAD_MASKED,   "SAD_MASKED" },
          offsetBufferId(MaskedSAD_Orig, 2),    //{ Take::DF_SAD_MASKED2,  "SAD_MASKED2" },
          offsetBufferId(MaskedSAD_Orig, 4),    //{ Take::DF_SAD_MASKED4,  "SAD_MASKED4" },
          offsetBufferId(MaskedSAD_Orig, 8),    //{ Take::DF_SAD_MASKED8,  "SAD_MASKED8" },
          offsetBufferId(MaskedSAD_Orig, 16),   //{ Take::DF_SAD_MASKED16, "SAD_MASKED16" },
          offsetBufferId(MaskedSAD_Orig, 32),   //{ Take::DF_SAD_MASKED32, "SAD_MASKED32" },
          offsetBufferId(MaskedSAD_Orig, 64),   //{ Take::DF_SAD_MASKED64, "SAD_MASKED64" },
          offsetBufferId(MaskedSAD_Orig, 128),  //{ Take::DF_SAD_MASKED128,"SAD_MASKED128" },

          offsetBufferId(FastHAD_Orig, 1),    //{ Take::DF_HAD_fast,        "HAD_fast" },
          offsetBufferId(FastHAD_Orig, 2),    //{ Take::DF_HAD2_fast,       "HAD2_fast" },
          offsetBufferId(FastHAD_Orig, 4),    //{ Take::DF_HAD4_fast,       "HAD4_fast" },
          offsetBufferId(FastHAD_Orig, 8),    //{ Take::DF_HAD8_fast,       "HAD8_fast" },
          offsetBufferId(FastHAD_Orig, 16),   //{ Take::DF_HAD16_fast,      "HAD16_fast" },
          offsetBufferId(FastHAD_Orig, 32),   //{ Take::DF_HAD32_fast,      "HAD32_fast" },
          offsetBufferId(FastHAD_Orig, 64),   //{ Take::DF_HAD64_fast,      "HAD64_fast" },
          offsetBufferId(FastHAD_Orig, 128),  //{ Take::DF_HAD128_fast,     "HAD128_fast" },
  
          -1,      //{ Take::DF_HAD_2SAD,        "HAD_2SAD" },

          -1,        //{ Take::DF_TOTAL_FUNCTIONS, "TOTAL_FUNCTIONS" },
          offsetBufferId(WeightedSSE_Orig, 1),    //{ Take::DF_SSE_WTD,         "SSE_WTD" }
          offsetBufferId(WeightedSSE_Orig, 2),    //{ Take::DF_SSE_WTD2,        "SSE_WTD" }
          offsetBufferId(WeightedSSE_Orig, 4),    //{ Take::DF_SSE_WTD4,        "SSE_WTD" }
          offsetBufferId(WeightedSSE_Orig, 8),    //{ Take::DF_SSE_WTD8,        "SSE_WTD" }
          offsetBufferId(WeightedSSE_Orig, 16),   //{ Take::DF_SSE_WTD16,       "SSE_WTD" }
          offsetBufferId(WeightedSSE_Orig, 32),   //{ Take::DF_SSE_WTD32,       "SSE_WTD" }
          offsetBufferId(WeightedSSE_Orig, 64),   //{ Take::DF_SSE_WTD64,       "SSE_WTD" }
          offsetBufferId(WeightedSSE_Orig, 128),  //{ Take::DF_SSE_WTD128,      "SSE_WTD"}

          offsetBufferId(SAD8XN_Orig, 1),         //{ Take::DF_SAD8XN,           "SAD8XN"},
          offsetBufferId(SAD8XN_Orig, 2),         //{ Take::DF_SAD8XN2,          "SAD8XN2"},
          offsetBufferId(SAD8XN_Orig, 4),         //{ Take::DF_SAD8XN4,          "SAD8XN4"},
          offsetBufferId(SAD8XN_Orig, 8),         //{ Take::DF_SAD8XN8,          "SAD8XN8"},
          offsetBufferId(SAD8XN_Orig, 16),        //{ Take::DF_SAD8XN16,         "SAD8XN16"},
          offsetBufferId(SAD8XN_Orig, 32),        //{ Take::DF_SAD8XN32,         "SAD8XN32"},
          offsetBufferId(SAD8XN_Orig, 64),        //{ Take::DF_SAD8XN64,         "SAD8XN64"},
          offsetBufferId(SAD8XN_Orig, 128),       //{ Take::DF_SAD8XN128,        "SAD8XN128"},

          offsetBufferId(SAD16XN_Orig, 1),        //{ Take::DF_SAD16XN,          "SAD16XN"},
          offsetBufferId(SAD16XN_Orig, 2),        //{ Take::DF_SAD16XN2,         "SAD16XN2"},
          offsetBufferId(SAD16XN_Orig, 4),        //{ Take::DF_SAD16XN4,         "SAD16XN4"},
          offsetBufferId(SAD16XN_Orig, 8),        //{ Take::DF_SAD16XN8,         "SAD16XN8"},
          offsetBufferId(SAD16XN_Orig, 16),       //{ Take::DF_SAD16XN16,        "SAD16XN16"},
          offsetBufferId(SAD16XN_Orig, 32),       //{ Take::DF_SAD16XN32,        "SAD16XN32"},
          offsetBufferId(SAD16XN_Orig, 64),       //{ Take::DF_SAD16XN64,        "SAD16XN64"},
          offsetBufferId(SAD16XN_Orig, 128)       //{ Take::DF_SAD16XN128,       "SAD16XN128"},
        };

        constexpr std::array<const int64_t, Take::DF_TOTAL_FUNCTIONS_ACTUAL> DFunc_Curr = {
          offsetBufferId(SSE_Curr, 1),      //{ Take::DF_SSE,             "SSE" },
          offsetBufferId(SSE_Curr, 2),      //{ Take::DF_SSE2,            "SSE2" },
          offsetBufferId(SSE_Curr, 4),      //{ Take::DF_SSE4,            "SSE4" },
          offsetBufferId(SSE_Curr, 8),      //{ Take::DF_SSE8,            "SSE8" },
          offsetBufferId(SSE_Curr, 16),     //{ Take::DF_SSE16,           "SSE16" },
          offsetBufferId(SSE_Curr, 32),     //{ Take::DF_SSE32,           "SSE32" },
          offsetBufferId(SSE_Curr, 64),     //{ Take::DF_SSE64,           "SSE64" },
          offsetBufferId(SSE_Curr, 128),    //{ Take::DF_SSE128,          "SSE128" },

          offsetBufferId(SAD_Curr, 1),      //{ Take::DF_SAD,             "SAD" },
          offsetBufferId(SAD_Curr, 2),      //{ Take::DF_SAD2,            "SAD2" },
          offsetBufferId(SAD_Curr, 4),      //{ Take::DF_SAD4,            "SAD4" },
          offsetBufferId(SAD_Curr, 8),      //{ Take::DF_SAD8,            "SAD8" },
          offsetBufferId(SAD_Curr, 16),     //{ Take::DF_SAD16,           "SAD16" },
          offsetBufferId(SAD_Curr, 32),     //{ Take::DF_SAD32,           "SAD32" },
          offsetBufferId(SAD_Curr, 64),     //{ Take::DF_SAD64,           "SAD64" },
          offsetBufferId(SAD_Curr, 128),    //{ Take::DF_SAD128,          "SAD128" },

          offsetBufferId(HAD_Curr, 1),      //{ Take::DF_HAD,             "HAD" },
          offsetBufferId(HAD_Curr, 2),      //{ Take::DF_HAD2,            "HAD2" },
          offsetBufferId(HAD_Curr, 4),      //{ Take::DF_HAD4,            "HAD4" },
          offsetBufferId(HAD_Curr, 8),      //{ Take::DF_HAD8,            "HAD8" },
          offsetBufferId(HAD_Curr, 16),     //{ Take::DF_HAD16,           "HAD16" },
          offsetBufferId(HAD_Curr, 32),     //{ Take::DF_HAD32,           "HAD32" },
          offsetBufferId(HAD_Curr, 64),     //{ Take::DF_HAD64,           "HAD64" },
          offsetBufferId(HAD_Curr, 128),    //{ Take::DF_HAD128,          "HAD128" },

          offsetBufferId(MaskedSAD_Curr, 1),    //{ Take::DF_SAD_MASKED,   "SAD_MASKED" },
          offsetBufferId(MaskedSAD_Curr, 2),    //{ Take::DF_SAD_MASKED2,  "SAD_MASKED2" },
          offsetBufferId(MaskedSAD_Curr, 4),    //{ Take::DF_SAD_MASKED4,  "SAD_MASKED4" },
          offsetBufferId(MaskedSAD_Curr, 8),    //{ Take::DF_SAD_MASKED8,  "SAD_MASKED8" },
          offsetBufferId(MaskedSAD_Curr, 16),   //{ Take::DF_SAD_MASKED16, "SAD_MASKED16" },
          offsetBufferId(MaskedSAD_Curr, 32),   //{ Take::DF_SAD_MASKED32, "SAD_MASKED32" },
          offsetBufferId(MaskedSAD_Curr, 64),   //{ Take::DF_SAD_MASKED64, "SAD_MASKED64" },
          offsetBufferId(MaskedSAD_Curr, 128),  //{ Take::DF_SAD_MASKED128,"SAD_MASKED128" },

          offsetBufferId(FastHAD_Curr, 1),    //{ Take::DF_HAD_fast,        "HAD_fast" },
          offsetBufferId(FastHAD_Curr, 2),    //{ Take::DF_HAD2_fast,       "HAD2_fast" },
          offsetBufferId(FastHAD_Curr, 4),    //{ Take::DF_HAD4_fast,       "HAD4_fast" },
          offsetBufferId(FastHAD_Curr, 8),    //{ Take::DF_HAD8_fast,       "HAD8_fast" },
          offsetBufferId(FastHAD_Curr, 16),   //{ Take::DF_HAD16_fast,      "HAD16_fast" },
          offsetBufferId(FastHAD_Curr, 32),   //{ Take::DF_HAD32_fast,      "HAD32_fast" },
          offsetBufferId(FastHAD_Curr, 64),   //{ Take::DF_HAD64_fast,      "HAD64_fast" },
          offsetBufferId(FastHAD_Curr, 128),  //{ Take::DF_HAD128_fast,     "HAD128_fast" },
  
          -1,      //{ Take::DF_HAD_2SAD,        "HAD_2SAD" },

          -1,        //{ Take::DF_TOTAL_FUNCTIONS, "TOTAL_FUNCTIONS" },
          offsetBufferId(WeightedSSE_Curr, 1),    //{ Take::DF_SSE_WTD,         "SSE_WTD" }
          offsetBufferId(WeightedSSE_Curr, 2),    //{ Take::DF_SSE_WTD2,        "SSE_WTD" }
          offsetBufferId(WeightedSSE_Curr, 4),    //{ Take::DF_SSE_WTD4,        "SSE_WTD" }
          offsetBufferId(WeightedSSE_Curr, 8),    //{ Take::DF_SSE_WTD8,        "SSE_WTD" }
          offsetBufferId(WeightedSSE_Curr, 16),   //{ Take::DF_SSE_WTD16,       "SSE_WTD" }
          offsetBufferId(WeightedSSE_Curr, 32),   //{ Take::DF_SSE_WTD32,       "SSE_WTD" }
          offsetBufferId(WeightedSSE_Curr, 64),   //{ Take::DF_SSE_WTD64,       "SSE_WTD" }
          offsetBufferId(WeightedSSE_Curr, 128),  //{ Take::DF_SSE_WTD128,      "SSE_WTD"}

          offsetBufferId(SAD8XN_Curr, 1),         //{ Take::DF_SAD8XN,           "SAD8XN"},
          offsetBufferId(SAD8XN_Curr, 2),         //{ Take::DF_SAD8XN2,          "SAD8XN2"},
          offsetBufferId(SAD8XN_Curr, 4),         //{ Take::DF_SAD8XN4,          "SAD8XN4"},
          offsetBufferId(SAD8XN_Curr, 8),         //{ Take::DF_SAD8XN8,          "SAD8XN8"},
          offsetBufferId(SAD8XN_Curr, 16),        //{ Take::DF_SAD8XN16,         "SAD8XN16"},
          offsetBufferId(SAD8XN_Curr, 32),        //{ Take::DF_SAD8XN32,         "SAD8XN32"},
          offsetBufferId(SAD8XN_Curr, 64),        //{ Take::DF_SAD8XN64,         "SAD8XN64"},
          offsetBufferId(SAD8XN_Curr, 128),       //{ Take::DF_SAD8XN128,        "SAD8XN128"},

          offsetBufferId(SAD16XN_Curr, 1),        //{ Take::DF_SAD16XN,          "SAD16XN"},
          offsetBufferId(SAD16XN_Curr, 2),        //{ Take::DF_SAD16XN2,         "SAD16XN2"},
          offsetBufferId(SAD16XN_Curr, 4),        //{ Take::DF_SAD16XN4,         "SAD16XN4"},
          offsetBufferId(SAD16XN_Curr, 8),        //{ Take::DF_SAD16XN8,         "SAD16XN8"},
          offsetBufferId(SAD16XN_Curr, 16),       //{ Take::DF_SAD16XN16,        "SAD16XN16"},
          offsetBufferId(SAD16XN_Curr, 32),       //{ Take::DF_SAD16XN32,        "SAD16XN32"},
          offsetBufferId(SAD16XN_Curr, 64),       //{ Take::DF_SAD16XN64,        "SAD16XN64"},
          offsetBufferId(SAD16XN_Curr, 128)       //{ Take::DF_SAD16XN128,       "SAD16XN128"},
        };


        /*constexpr int64_t RECO_MOTION_ESTIMATION           = 0;
        constexpr int64_t RECO_AFFINE_MOTION_ESTIMATION       = 1;
        constexpr int64_t ORIG_MOTION_ESTIMATION           = 2;
        constexpr int64_t TEMP_ORIG_MOTION_ESTIMATION         = 11;
        constexpr int64_t ORIG_AFFINE_MOTION_ESTIMATION       = 3;
        constexpr int64_t TEMP_ORIG_AFFINE_MOTION_ESTIMATION    = 13;*/
        //constexpr int64_t FILT_MOTION_ESTIMATION_TEMP       = 4;
        //constexpr int64_t FILT_MOTION_ESTIMATION           = 5;
        /*constexpr int64_t PRED_AFFINE_MOTION_ESTIMATION       = 6;*/

        //constexpr int64_t RECO_MOTION_ESTIMATION_MVP         = 12;
        //constexpr int64_t RECO_MOTION_ESTIMATION_PATTERN       = 7;
        /*constexpr int64_t RECO_MOTION_ESTIMATION_MVP_AND_PATTERN  = 7;
        constexpr int64_t RECO_MOTION_ESTIMATION_TZ         = 8;
        constexpr int64_t RECO_MOTION_ESTIMATION_FAST         = 9;
        constexpr int64_t RECO_MOTION_ESTIMATION_FRACTIONAL     = 10;
        constexpr int64_t RECO_MOTION_ESTIMATION_REFINEMENT     = 14;

        constexpr int64_t ORIG_MOTION_ESTIMATION_IME         = 15;
        constexpr int64_t ORIG_MOTION_ESTIMATION_FRACTIONAL     = 16;
        constexpr int64_t ORIG_MOTION_ESTIMATION_REFINEMENT     = 17;

        constexpr int64_t FME_RECO                   = RECO_MOTION_ESTIMATION_FRACTIONAL;
        constexpr int64_t FME_ORIG                   = ORIG_MOTION_ESTIMATION_FRACTIONAL;*/

        /*constexpr int64_t FME_RECO_HR                = 20;
        constexpr int64_t FME_RECO_QR                = 21;
        constexpr int64_t FME_ORIG_HR                = 22;
        constexpr int64_t FME_ORIG_QR                = 23;*/

        /*static constexpr auto FME_FILT_OFFSET            = "1";
        static constexpr auto FME_FILT_TEMP_OFFSET          = "20";


        extern int64_t FME_FILT      [vvenc::LUMA_INTERPOLATION_FILTER_SUB_SAMPLE_POSITIONS_SIGNAL][vvenc::LUMA_INTERPOLATION_FILTER_SUB_SAMPLE_POSITIONS_SIGNAL][vvenc::ComponentID::MAX_NUM_COMP];
        extern int64_t FME_FILT_TEMP  [vvenc::LUMA_INTERPOLATION_FILTER_SUB_SAMPLE_POSITIONS_SIGNAL][vvenc::ComponentID::MAX_NUM_COMP];*/
      }

      namespace LevelId {
        constexpr int64_t OTHER = 10;
        constexpr int64_t xCheckRDCostMerge = 11;
          constexpr int64_t xCheckRDCostMerge_start = 46;
          constexpr int64_t xCheckRDCostMerge_SATDCost = 47;
          constexpr int64_t xCheckRDCostMerge_RDChecking = 48;
          constexpr int64_t generateMergePrediction = 49;
          constexpr int64_t generateMergePrediction_REGULAR = 80;
          constexpr int64_t generateMergePrediction_CIIP = 81;
          constexpr int64_t generateMergePrediction_MMVD = 82;
          constexpr int64_t generateMergePrediction_SBTMVP = 83;
          constexpr int64_t generateMergePrediction_AFFINE = 84;
          constexpr int64_t generateMergePrediction_GPM = 85;

          constexpr int64_t addCandsToPruningList_REGULAR = 86;
          constexpr int64_t addCandsToPruningList_CIIP = 87;
          constexpr int64_t addCandsToPruningList_MMVD = 88;
          constexpr int64_t addCandsToPruningList_AFFINE = 89;
          constexpr int64_t addCandsToPruningList_GPM = 90;

          constexpr int64_t merge_RD_copyFrom = 91; //last added

        constexpr int64_t xCheckRDCostMergeGeo = 12;
        constexpr int64_t xCheckRDCostIBCModeMerge2Nx2N = 14;
        constexpr int64_t xCheckRDCostIBCMode = 15;
        constexpr int64_t xCheckRDCostIntra = 16;
          constexpr int64_t estIntraPredLumaQT = 50;
            constexpr int64_t xIntraCodingLumaQT = 51;
            constexpr int64_t xIntraChromaCodingQT = 52;
            constexpr int64_t predIntraChromaLM = 53;
            constexpr int64_t predIntraAng = 54;
            constexpr int64_t xIntraCodingTUBlock = 55;
          constexpr int64_t estIntraPredChromaQT = 51;

        constexpr int64_t xCheckRDCostInterIMV = 13;
        constexpr int64_t xCheckRDCostInter = 17;
          constexpr int64_t xEncodeInterResidual = 20;
          constexpr int64_t predInterSearch = 21;
          constexpr int64_t xEstimateMvPredAMVP = 70;
          constexpr int64_t xGetTemplateCost = 71;
            constexpr int64_t xMotionEstimation = 22;
              constexpr int64_t xMotionEstimation_removeHighFreq = 75; 
              constexpr int64_t Full_Search_MPV = 23;
              constexpr int64_t xPatternSearch = 24;
              constexpr int64_t xTZSearch = 25;
                constexpr int64_t xTZSearch_MVP = 26;
                constexpr int64_t xTZSearch_Start_Search = 27;
                constexpr int64_t xTZSearch_Raster = 28;
                constexpr int64_t xTZSearch_RasterRefinement = 45; 
                constexpr int64_t xTZSearch_Refinement = 29;
              constexpr int64_t xPatternSearchFracDIF = 30;
                constexpr int64_t xPatternSearchFracDIF_HalfPixel = 31;
                  constexpr int64_t xExtDIFUpSamplingH = 61;
                constexpr int64_t xPatternSearchFracDIF_QuarterPixel = 32;
                  constexpr int64_t xExtDIFUpSamplingQ = 62;
                
                constexpr int64_t m_fastSubPel = 63;

              constexpr int64_t xPatternSearchIntRefine = 33;

            constexpr int64_t xPredAffineInterSearch = 34;  //DO NOT TRUST FOR JICS 2025, WAS NOT PROPERLY MARKEK
              constexpr int64_t predInterSearch_AFFINEMODEL_4PARAM = 72;
              constexpr int64_t predInterSearch_AFFINEMODEL_6PARAM = 73;
              constexpr int64_t xAffineMotionEstimation = 35;
              constexpr int64_t xAffineMotionEstimation_removeHighFreq = 74;

          constexpr int64_t motionCompensation = 36;
            constexpr int64_t xPredInterUni = 37;
              constexpr int64_t xPredAffineBlk = 38;   //DO NOT TRUST FOR JICS 2025, WAS NOT PROPERLY MARKEK
              constexpr int64_t xPredInterBlk = 39;
            constexpr int64_t xPredInterBi = 40;
              constexpr int64_t xWeightedAverage = 41;
            constexpr int64_t xSubPuBDOF = 42;
            constexpr int64_t xProcessDMVR = 43;
            
          constexpr int64_t motionCompensationIBC = 44;
        
        constexpr int64_t xCalDebCost = 18;

        constexpr int64_t MSE = 96;
        constexpr int64_t SSE = 97;
        constexpr int64_t WeightedSSE = 101;
        constexpr int64_t SAD = 98;
        constexpr int64_t MaskedSAD = 100; 
        constexpr int64_t HAD = 99;
        constexpr int64_t FastHAD = 102;
        constexpr int64_t HAD_2SAD = 103;

        constexpr std::array<const int64_t, Take::DF_TOTAL_FUNCTIONS_ACTUAL> DFunc = {
          SSE,   //{ Take::DF_SSE,             "SSE" },
          SSE,   //{ Take::DF_SSE2,            "SSE2" },
          SSE,   //{ Take::DF_SSE4,            "SSE4" },
          SSE,   //{ Take::DF_SSE8,            "SSE8" },
          SSE,   //{ Take::DF_SSE16,           "SSE16" },
          SSE,   //{ Take::DF_SSE32,           "SSE32" },
          SSE,   //{ Take::DF_SSE64,           "SSE64" },
          SSE,   //{ Take::DF_SSE128,          "SSE128" },

          SAD,   //{ Take::DF_SAD,             "SAD" },
          SAD,   //{ Take::DF_SAD2,            "SAD2" },
          SAD,   //{ Take::DF_SAD4,            "SAD4" },
          SAD,   //{ Take::DF_SAD8,            "SAD8" },
          SAD,   //{ Take::DF_SAD16,           "SAD16" },
          SAD,   //{ Take::DF_SAD32,           "SAD32" },
          SAD,   //{ Take::DF_SAD64,           "SAD64" },
          SAD,   //{ Take::DF_SAD128,          "SAD128" },

          HAD,  //{ Take::DF_HAD,             "HAD" },
          HAD,  //{ Take::DF_HAD2,            "HAD2" },
          HAD,  //{ Take::DF_HAD4,            "HAD4" },
          HAD,  //{ Take::DF_HAD8,            "HAD8" },
          HAD,  //{ Take::DF_HAD16,           "HAD16" },
          HAD,  //{ Take::DF_HAD32,           "HAD32" },
          HAD,  //{ Take::DF_HAD64,           "HAD64" },
          HAD,  //{ Take::DF_HAD128,          "HAD128" },

          MaskedSAD, //{ Take::DF_SAD_MASKED,   "SAD_MASKED" },
          MaskedSAD, //{ Take::DF_SAD_MASKED2,  "SAD_MASKED2" },
          MaskedSAD, //{ Take::DF_SAD_MASKED4,  "SAD_MASKED4" },
          MaskedSAD, //{ Take::DF_SAD_MASKED8,  "SAD_MASKED8" },
          MaskedSAD, //{ Take::DF_SAD_MASKED16, "SAD_MASKED16" },
          MaskedSAD, //{ Take::DF_SAD_MASKED32, "SAD_MASKED32" },
          MaskedSAD, //{ Take::DF_SAD_MASKED64, "SAD_MASKED64" },
          MaskedSAD, //{ Take::DF_SAD_MASKED128,"SAD_MASKED128" },

          FastHAD, //{ Take::DF_HAD_fast,        "HAD_fast" },
          FastHAD, //{ Take::DF_HAD2_fast,       "HAD2_fast" },
          FastHAD, //{ Take::DF_HAD4_fast,       "HAD4_fast" },
          FastHAD, //{ Take::DF_HAD8_fast,       "HAD8_fast" },
          FastHAD, //{ Take::DF_HAD16_fast,      "HAD16_fast" },
          FastHAD, //{ Take::DF_HAD32_fast,      "HAD32_fast" },
          FastHAD, //{ Take::DF_HAD64_fast,      "HAD64_fast" },
          FastHAD, //{ Take::DF_HAD128_fast,     "HAD128_fast" },

          HAD_2SAD,      //{ Take::DF_HAD_2SAD,        "HAD_2SAD" },

          -1,        //{ Take::DF_TOTAL_FUNCTIONS, "TOTAL_FUNCTIONS" },
          WeightedSSE,  //{ Take::DF_SSE_WTD,         "SSE_WTD" }
          WeightedSSE,  //{ Take::DF_SSE_WTD2,        "SSE_WTD" }
          WeightedSSE,  //{ Take::DF_SSE_WTD4,        "SSE_WTD" }
          WeightedSSE,  //{ Take::DF_SSE_WTD8,        "SSE_WTD" }
          WeightedSSE,  //{ Take::DF_SSE_WTD16,       "SSE_WTD" }
          WeightedSSE,  //{ Take::DF_SSE_WTD32,       "SSE_WTD" }
          WeightedSSE,  //{ Take::DF_SSE_WTD64,       "SSE_WTD" }
          WeightedSSE,  //{ Take::DF_SSE_WTD128,      "SSE_WTD"}

          SAD,          //{ Take::DF_SAD8XN,           "SAD8XN"},
          SAD,          //{ Take::DF_SAD8XN2,          "SAD8XN2"},
          SAD,          //{ Take::DF_SAD8XN4,          "SAD8XN4"},
          SAD,          //{ Take::DF_SAD8XN8,          "SAD8XN8"},
          SAD,          //{ Take::DF_SAD8XN16,         "SAD8XN16"},
          SAD,          //{ Take::DF_SAD8XN32,         "SAD8XN32"},
          SAD,          //{ Take::DF_SAD8XN64,         "SAD8XN64"},
          SAD,          //{ Take::DF_SAD8XN128,        "SAD8XN128"},
          SAD,          //{ Take::DF_SAD16XN,          "SAD16XN"},
          SAD,          //{ Take::DF_SAD16XN2,         "SAD16XN2"},
          SAD,          //{ Take::DF_SAD16XN4,         "SAD16XN4"},
          SAD,          //{ Take::DF_SAD16XN8,         "SAD16XN8"},
          SAD,          //{ Take::DF_SAD16XN16,        "SAD16XN16"},
          SAD,          //{ Take::DF_SAD16XN32,        "SAD16XN32"},
          SAD,          //{ Take::DF_SAD16XN64,        "SAD16XN64"},
          SAD           //{ Take::DF_SAD16XN128,       "SAD16XN128"},
        };

      }

      namespace ConfigurationId {
        static constexpr int64_t JUST_TRACKING     = 0;
        static constexpr int64_t PRECISE_KNOB      = 2;
        static constexpr int64_t APPROXIMATE_KNOB  = 1;

        constexpr int64_t OTHER =             JUST_TRACKING;

        constexpr int64_t SAD_Orig =          APPROXIMATE_KNOB;
        constexpr int64_t SAD_Curr =          APPROXIMATE_KNOB;

        constexpr int64_t MaskedSAD_Orig =    APPROXIMATE_KNOB;
        constexpr int64_t MaskedSAD_Curr =    APPROXIMATE_KNOB;

        constexpr int64_t SSE_Orig =          APPROXIMATE_KNOB;
        constexpr int64_t SSE_Curr =          APPROXIMATE_KNOB;

        constexpr int64_t WeightedSSE_Orig =  APPROXIMATE_KNOB;
        constexpr int64_t WeightedSSE_Curr =  APPROXIMATE_KNOB;

        constexpr int64_t HAD_Orig =          APPROXIMATE_KNOB;
        constexpr int64_t HAD_Curr =          APPROXIMATE_KNOB;

        constexpr int64_t FastHAD_Orig =      APPROXIMATE_KNOB;
        constexpr int64_t FastHAD_Curr =      APPROXIMATE_KNOB;

        constexpr int64_t HAD_2SAD_Orig =     APPROXIMATE_KNOB;
        constexpr int64_t HAD_2SAD_Curr =     APPROXIMATE_KNOB;

        constexpr std::array<const int64_t, Take::DF_TOTAL_FUNCTIONS_ACTUAL> DFunc_Orig = {
          SSE_Orig,   //{ Take::DF_SSE,             "SSE" },
          SSE_Orig,   //{ Take::DF_SSE2,            "SSE2" },
          SSE_Orig,   //{ Take::DF_SSE4,            "SSE4" },
          SSE_Orig,   //{ Take::DF_SSE8,            "SSE8" },
          SSE_Orig,   //{ Take::DF_SSE16,           "SSE16" },
          SSE_Orig,   //{ Take::DF_SSE32,           "SSE32" },
          SSE_Orig,   //{ Take::DF_SSE64,           "SSE64" },
          SSE_Orig,   //{ Take::DF_SSE128,          "SSE128" },

          SAD_Orig,   //{ Take::DF_SAD,             "SAD" },
          SAD_Orig,   //{ Take::DF_SAD2,            "SAD2" },
          SAD_Orig,   //{ Take::DF_SAD4,            "SAD4" },
          SAD_Orig,   //{ Take::DF_SAD8,            "SAD8" },
          SAD_Orig,   //{ Take::DF_SAD16,           "SAD16" },
          SAD_Orig,   //{ Take::DF_SAD32,           "SAD32" },
          SAD_Orig,   //{ Take::DF_SAD64,           "SAD64" },
          SAD_Orig,   //{ Take::DF_SAD128,          "SAD128" },

          HAD_Orig,  //{ Take::DF_HAD,             "HAD" },
          HAD_Orig,  //{ Take::DF_HAD2,            "HAD2" },
          HAD_Orig,  //{ Take::DF_HAD4,            "HAD4" },
          HAD_Orig,  //{ Take::DF_HAD8,            "HAD8" },
          HAD_Orig,  //{ Take::DF_HAD16,           "HAD16" },
          HAD_Orig,  //{ Take::DF_HAD32,           "HAD32" },
          HAD_Orig,  //{ Take::DF_HAD64,           "HAD64" },
          HAD_Orig,  //{ Take::DF_HAD128,          "HAD128" },

          MaskedSAD_Orig, //{ Take::DF_SAD_MASKED,   "SAD_MASKED" },
          MaskedSAD_Orig, //{ Take::DF_SAD_MASKED2,  "SAD_MASKED2" },
          MaskedSAD_Orig, //{ Take::DF_SAD_MASKED4,  "SAD_MASKED4" },
          MaskedSAD_Orig, //{ Take::DF_SAD_MASKED8,  "SAD_MASKED8" },
          MaskedSAD_Orig, //{ Take::DF_SAD_MASKED16, "SAD_MASKED16" },
          MaskedSAD_Orig, //{ Take::DF_SAD_MASKED32, "SAD_MASKED32" },
          MaskedSAD_Orig, //{ Take::DF_SAD_MASKED64, "SAD_MASKED64" },
          MaskedSAD_Orig, //{ Take::DF_SAD_MASKED128,"SAD_MASKED128" },

          FastHAD_Orig, //{ Take::DF_HAD_fast,        "HAD_fast" },
          FastHAD_Orig, //{ Take::DF_HAD2_fast,       "HAD2_fast" },
          FastHAD_Orig, //{ Take::DF_HAD4_fast,       "HAD4_fast" },
          FastHAD_Orig, //{ Take::DF_HAD8_fast,       "HAD8_fast" },
          FastHAD_Orig, //{ Take::DF_HAD16_fast,      "HAD16_fast" },
          FastHAD_Orig, //{ Take::DF_HAD32_fast,      "HAD32_fast" },
          FastHAD_Orig, //{ Take::DF_HAD64_fast,      "HAD64_fast" },
          FastHAD_Orig, //{ Take::DF_HAD128_fast,     "HAD128_fast" },

          HAD_2SAD_Orig,  //{ Take::DF_HAD_2SAD,        "HAD_2SAD" }, // PREFERABLY NOT TO BE USED

          OTHER,        //{ Take::DF_TOTAL_FUNCTIONS, "TOTAL_FUNCTIONS" },
          WeightedSSE_Orig,  //{ Take::DF_SSE_WTD,         "SSE_WTD" }
          WeightedSSE_Orig,  //{ Take::DF_SSE_WTD2,         "SSE_WTD" }
          WeightedSSE_Orig,  //{ Take::DF_SSE_WTD4,         "SSE_WTD" }
          WeightedSSE_Orig,  //{ Take::DF_SSE_WTD8,         "SSE_WTD" }
          WeightedSSE_Orig,  //{ Take::DF_SSE_WTD16,        "SSE_WTD" }
          WeightedSSE_Orig,  //{ Take::DF_SSE_WTD32,        "SSE_WTD" }
          WeightedSSE_Orig,  //{ Take::DF_SSE_WTD64,        "SSE_WTD" }
          WeightedSSE_Orig,  //{ Take::DF_SSE_WTD128,       "SSE_WTD" }

          SAD_Orig,          //{ Take::DF_SAD8XN,           "SAD8XN"},
          SAD_Orig,          //{ Take::DF_SAD8XN2,          "SAD8XN2"},
          SAD_Orig,          //{ Take::DF_SAD8XN4,          "SAD8XN4"},
          SAD_Orig,          //{ Take::DF_SAD8XN8,          "SAD8XN8"},
          SAD_Orig,          //{ Take::DF_SAD8XN16,         "SAD8XN16"},
          SAD_Orig,          //{ Take::DF_SAD8XN32,         "SAD8XN32"},
          SAD_Orig,          //{ Take::DF_SAD8XN64,         "SAD8XN64"},
          SAD_Orig,          //{ Take::DF_SAD8XN128,        "SAD8XN128"},

          SAD_Orig,          //{ Take::DF_SAD16XN,          "SAD16XN"},
          SAD_Orig,          //{ Take::DF_SAD16XN2,         "SAD16XN2"},
          SAD_Orig,          //{ Take::DF_SAD16XN4,         "SAD16XN4"},
          SAD_Orig,          //{ Take::DF_SAD16XN8,         "SAD16XN8"},
          SAD_Orig,          //{ Take::DF_SAD16XN16,        "SAD16XN16"},
          SAD_Orig,          //{ Take::DF_SAD16XN32,        "SAD16XN32"},
          SAD_Orig,          //{ Take::DF_SAD16XN64,        "SAD16XN64"},
          SAD_Orig           //{ Take::DF_SAD16XN128,       "SAD16XN128"},
        };

        constexpr std::array<const int64_t, Take::DF_TOTAL_FUNCTIONS_ACTUAL> DFunc_Curr = {
          SSE_Curr,   //{ Take::DF_SSE,             "SSE" },
          SSE_Curr,   //{ Take::DF_SSE2,            "SSE2" },
          SSE_Curr,   //{ Take::DF_SSE4,            "SSE4" },
          SSE_Curr,   //{ Take::DF_SSE8,            "SSE8" },
          SSE_Curr,   //{ Take::DF_SSE16,           "SSE16" },
          SSE_Curr,   //{ Take::DF_SSE32,           "SSE32" },
          SSE_Curr,   //{ Take::DF_SSE64,           "SSE64" },
          SSE_Curr,   //{ Take::DF_SSE128,          "SSE128" },

          SAD_Curr,   //{ Take::DF_SAD,             "SAD" },
          SAD_Curr,   //{ Take::DF_SAD2,            "SAD2" },
          SAD_Curr,   //{ Take::DF_SAD4,            "SAD4" },
          SAD_Curr,   //{ Take::DF_SAD8,            "SAD8" },
          SAD_Curr,   //{ Take::DF_SAD16,           "SAD16" },
          SAD_Curr,   //{ Take::DF_SAD32,           "SAD32" },
          SAD_Curr,   //{ Take::DF_SAD64,           "SAD64" },
          SAD_Curr,   //{ Take::DF_SAD128,          "SAD128" },

          HAD_Curr,  //{ Take::DF_HAD,             "HAD" },
          HAD_Curr,  //{ Take::DF_HAD2,            "HAD2" },
          HAD_Curr,  //{ Take::DF_HAD4,            "HAD4" },
          HAD_Curr,  //{ Take::DF_HAD8,            "HAD8" },
          HAD_Curr,  //{ Take::DF_HAD16,           "HAD16" },
          HAD_Curr,  //{ Take::DF_HAD32,           "HAD32" },
          HAD_Curr,  //{ Take::DF_HAD64,           "HAD64" },
          HAD_Curr,  //{ Take::DF_HAD128,          "HAD128" },

          MaskedSAD_Curr, //{ Take::DF_SAD_MASKED,   "SAD_MASKED" },
          MaskedSAD_Curr, //{ Take::DF_SAD_MASKED2,  "SAD_MASKED2" },
          MaskedSAD_Curr, //{ Take::DF_SAD_MASKED4,  "SAD_MASKED4" },
          MaskedSAD_Curr, //{ Take::DF_SAD_MASKED8,  "SAD_MASKED8" },
          MaskedSAD_Curr, //{ Take::DF_SAD_MASKED16, "SAD_MASKED16" },
          MaskedSAD_Curr, //{ Take::DF_SAD_MASKED32, "SAD_MASKED32" },
          MaskedSAD_Curr, //{ Take::DF_SAD_MASKED64, "SAD_MASKED64" },
          MaskedSAD_Curr, //{ Take::DF_SAD_MASKED128,"SAD_MASKED128" },

          FastHAD_Curr, //{ Take::DF_HAD_fast,        "HAD_fast" },
          FastHAD_Curr, //{ Take::DF_HAD2_fast,       "HAD2_fast" },
          FastHAD_Curr, //{ Take::DF_HAD4_fast,       "HAD4_fast" },
          FastHAD_Curr, //{ Take::DF_HAD8_fast,       "HAD8_fast" },
          FastHAD_Curr, //{ Take::DF_HAD16_fast,      "HAD16_fast" },
          FastHAD_Curr, //{ Take::DF_HAD32_fast,      "HAD32_fast" },
          FastHAD_Curr, //{ Take::DF_HAD64_fast,      "HAD64_fast" },
          FastHAD_Curr, //{ Take::DF_HAD128_fast,     "HAD128_fast" },

          HAD_2SAD_Curr,  //{ Take::DF_HAD_2SAD,        "HAD_2SAD" }, // PREFERABLY NOT TO BE USED

          OTHER,        //{ Take::DF_TOTAL_FUNCTIONS, "TOTAL_FUNCTIONS" },
          WeightedSSE_Curr,  //{ Take::DF_SSE_WTD,         "SSE_WTD" }
          WeightedSSE_Curr,  //{ Take::DF_SSE_WTD2,         "SSE_WTD" }
          WeightedSSE_Curr,  //{ Take::DF_SSE_WTD4,         "SSE_WTD" }
          WeightedSSE_Curr,  //{ Take::DF_SSE_WTD8,         "SSE_WTD" }
          WeightedSSE_Curr,  //{ Take::DF_SSE_WTD16,        "SSE_WTD" }
          WeightedSSE_Curr,  //{ Take::DF_SSE_WTD32,        "SSE_WTD" }
          WeightedSSE_Curr,  //{ Take::DF_SSE_WTD64,        "SSE_WTD" }
          WeightedSSE_Curr,  //{ Take::DF_SSE_WTD128,       "SSE_WTD" }

          SAD_Curr,          //{ Take::DF_SAD8XN,           "SAD8XN"},
          SAD_Curr,          //{ Take::DF_SAD8XN2,          "SAD8XN2"},
          SAD_Curr,          //{ Take::DF_SAD8XN4,          "SAD8XN4"},
          SAD_Curr,          //{ Take::DF_SAD8XN8,          "SAD8XN8"},
          SAD_Curr,          //{ Take::DF_SAD8XN16,         "SAD8XN16"},
          SAD_Curr,          //{ Take::DF_SAD8XN32,         "SAD8XN32"},
          SAD_Curr,          //{ Take::DF_SAD8XN64,         "SAD8XN64"},
          SAD_Curr,          //{ Take::DF_SAD8XN128,        "SAD8XN128"},

          SAD_Curr,          //{ Take::DF_SAD16XN,          "SAD16XN"},
          SAD_Curr,          //{ Take::DF_SAD16XN2,         "SAD16XN2"},
          SAD_Curr,          //{ Take::DF_SAD16XN4,         "SAD16XN4"},
          SAD_Curr,          //{ Take::DF_SAD16XN8,         "SAD16XN8"},
          SAD_Curr,          //{ Take::DF_SAD16XN16,        "SAD16XN16"},
          SAD_Curr,          //{ Take::DF_SAD16XN32,        "SAD16XN32"},
          SAD_Curr,          //{ Take::DF_SAD16XN64,        "SAD16XN64"},
          SAD_Curr           //{ Take::DF_SAD16XN128,       "SAD16XN128"},
        };

        /*constexpr int64_t RECO_MOTION_ESTIMATION           = JUST_TRACKING;
        constexpr int64_t RECO_AFFINE_MOTION_ESTIMATION       = JUST_TRACKING;
        constexpr int64_t ORIG_MOTION_ESTIMATION           = JUST_TRACKING;
        constexpr int64_t TEMP_ORIG_MOTION_ESTIMATION         = JUST_TRACKING;
        constexpr int64_t ORIG_AFFINE_MOTION_ESTIMATION       = JUST_TRACKING;
        constexpr int64_t TEMP_ORIG_AFFINE_MOTION_ESTIMATION    = JUST_TRACKING;
        constexpr int64_t FILT_MOTION_ESTIMATION_TEMP         = JUST_TRACKING;
        constexpr int64_t FILT_MOTION_ESTIMATION           = JUST_TRACKING;
        constexpr int64_t PRED_AFFINE_MOTION_ESTIMATION       = JUST_TRACKING;*/

        //constexpr int64_t RECO_MOTION_ESTIMATION_MVP         = 0;
        //constexpr int64_t RECO_MOTION_ESTIMATION_PATTERN       = 0;
        /*constexpr int64_t RECO_MOTION_ESTIMATION_MVP_AND_PATTERN  = JUST_TRACKING;
        constexpr int64_t RECO_MOTION_ESTIMATION_TZ         = JUST_TRACKING;
        constexpr int64_t RECO_MOTION_ESTIMATION_FAST         = JUST_TRACKING;
        constexpr int64_t RECO_MOTION_ESTIMATION_FRACTIONAL     = JUST_TRACKING;
        constexpr int64_t RECO_MOTION_ESTIMATION_REFINEMENT     = JUST_TRACKING;

        constexpr int64_t ORIG_MOTION_ESTIMATION_IME         = JUST_TRACKING;
        constexpr int64_t ORIG_MOTION_ESTIMATION_FRACTIONAL     = JUST_TRACKING;
        constexpr int64_t ORIG_MOTION_ESTIMATION_REFINEMENT     = JUST_TRACKING;

        constexpr int64_t FME_RECO                   = RECO_MOTION_ESTIMATION_FRACTIONAL;
        constexpr int64_t FME_ORIG                   = ORIG_MOTION_ESTIMATION_FRACTIONAL;*/

        /*constexpr int64_t FME_RECO_HR                = JUST_TRACKING;
        constexpr int64_t FME_RECO_QR                = JUST_TRACKING;
        constexpr int64_t FME_ORIG_HR                = JUST_TRACKING;
        constexpr int64_t FME_ORIG_QR                = JUST_TRACKING;*/

        /*constexpr int64_t FME_FILT                  = JUST_TRACKING;  
        constexpr int64_t FME_FILT_TEMP                = JUST_TRACKING;*/
      }


      #if PRINT_COST
        extern double bestTempCost;
      #endif


      void MarkBuffer(const BufferRange& toMark);
      void MarkBuffer(void * const initialAddress, void const * const endAddress, const int64_t bufferId, const int64_t configurationId, const uint32_t dataSizeInBytes);
      void RemarkBuffer(void * const initialAddress/*, void const * const endAddress*/, const int64_t bufferId, const int64_t configurationId, const uint32_t dataSizeInBytes);
      void UnmarkBuffer(const BufferRange& toUnmark);
      void UnmarkBuffer(void const * const address);
      void UnmarkBuffer(void const * const start_address, void const * const endAddress);
      void InstrumentIfMarked(void * const address, const int64_t bufferId, const int64_t configurationId, const uint32_t dataSizeInBytes);
      void InstrumentIfMarked(void * const address, const int64_t bufferIdPrefix, const int64_t configurationId);
      void ReinstrumentIfMarked(void * const address, const int64_t bufferId, const int64_t configurationId, const uint32_t dataSizeInBytes);
      void UninstrumentIfMarked(void * const address, const bool giveAwayRecords = true);

      void PrintMacrosStates();
      void PrintMacroState(const std::string& macroName, const bool macroStatus, const std::string& tab = "\t");

      void PrintBuffersInfo();
      void PrintBufferInfo(const std::string& bufferName, const int64_t bufferId, const int64_t configurationId, const std::string& tab = "\t");

      #if COST_CAPTURE
      void ProcessTake(const uint8_t takeId, const uint8_t funcId, const uint64_t cost);
      #endif

    #if FELIPE_INSTRUMENTATION
      #if APPROX_RECO_BUFFER_INTER
        namespace RECO { 
          //public:
          extern std::fstream fp;
          extern int debugEnable;

          //double MEReadBER, MEWriteBER;
          extern unsigned frameBufferWidth, frameBufferHeight;
          extern unsigned xMargin, yMargin;
          extern bool collectBufferSize, tmpBool;

          void init();
          void initDebug();
        }
      #endif

      #if APPROX_ORIG_BUFFER_INTER
        namespace ORIG {
          //public:
          extern std::fstream fp;
          extern int debugEnable;

          //double MEReadBER, MEWriteBER;
          extern unsigned frameOrigBufferWidth, frameOrigBufferHeight;
          ////unsigned xMargin, yMargin;
          extern bool collectBufferSize, tmpBool;

          void init();
          void initDebug();
        }
      #endif

      #if APPROX_FILT_BUFFER_V1 || APPROX_FILT_BUFFER_V2
        namespace FILT {
          //<Felipe>
          extern int extWidthFiltered, extHeightFiltered;
          //</Felipe>
        }
      #endif
    #endif
  }

#endif