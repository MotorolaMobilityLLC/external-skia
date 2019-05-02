package mtkJpegEnhanceSkia
import (
	"android/soong/android"
	"android/soong/cc"
	//"android/soong/android/mediatek"
	//"github.com/google/blueprint"
)

func mtkJpegEnhanceSkiaDefaults(ctx android.LoadHookContext) {
	type props struct {
		Target struct {
			Android struct {
				Cflags []string
				Local_include_dirs []string
				Include_dirs []string
				Srcs []string
				//Legacy_shared_libs []string
			}
		}
	}
	p := &props{}
	//featureValue := android.MtkFeatureValues

	var localIncludeDirs []string
	var includeDirs []string
	var srcs []string
	vars := ctx.Config().VendorConfig("mtkPlugin")
	targetBoardPlatform := vars.String("TARGET_BOARD_PLATFORM")
	//var legacysharedlibs []string

			//graphics flags
	switch targetBoardPlatform {
		case "mt6771","mt6765","mt6779","mt6768","mt6761":
			p.Target.Android.Cflags = append(p.Target.Android.Cflags, "-D__MTK_TRACE_SKIA__")
			p.Target.Android.Cflags = append(p.Target.Android.Cflags, "-D__MTK_TRACE_MT_BLITTER__")
			//p.Target.Android.Cflags = append(p.Target.Android.Cflags, "-D__MTK_AFFINE_TRANSFORM_EXT__")
			p.Target.Android.Cflags = append(p.Target.Android.Cflags, "-D__MULTI_THREADS_OPTIMIZE_2D__")
			//p.Target.Android.Cflags = append(p.Target.Android.Cflags, "-D__MTK_DUMP_DRAW_BITMAP__")
}
			//graphics local dirs
			localIncludeDirs = append(localIncludeDirs, "mtk_opt/include/adapter")
			localIncludeDirs = append(localIncludeDirs, "mtk_opt/src/adapter")

			//graphics srcs
			srcs = append(srcs, "mtk_opt/src/adapter/SkBlitterAdapterHandler.cpp")
			srcs = append(srcs, "mtk_opt/src/adapter/SkBlitterMTAdapter.cpp")




	p.Target.Android.Local_include_dirs = localIncludeDirs
	p.Target.Android.Include_dirs = includeDirs
	p.Target.Android.Srcs = srcs
	//p.Target.Android.Legacy_shared_libs = legacysharedlibs

	ctx.AppendProperties(p)
}

func init() {
	android.RegisterModuleType("mtk_jpeg_enhance_skia_defaults", mtkJpegEnhanceSkiaDefaultsFactory)
}

func mtkJpegEnhanceSkiaDefaultsFactory() (android.Module) {
	module := cc.DefaultsFactory()
	android.AddLoadHook(module, mtkJpegEnhanceSkiaDefaults)
	return module
}