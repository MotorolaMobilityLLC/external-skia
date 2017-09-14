package mtkJpegEnhance
import (
	"android/soong/android"
	"android/soong/cc"
	"github.com/google/blueprint"
)
 
func mtkJpegEnhanceDefaults(ctx android.LoadHookContext) {
	type props struct {
		Target struct {
			Android struct {
				Cflags []string
				Include_dirs []string
				Legacy_shared_libs []string
			}
		}
	}
	p := &props{}
	featureValue := android.MtkFeatureValues

	var includeDirs []string
	var legacysharedlibs []string

	if v, found := featureValue["MTK_JPEG_HW_RESIZER_TYPE"]; found {
		if v == "HW_RESIZER_TYPE_2" { 
			p.Target.Android.Cflags = append(p.Target.Android.Cflags, "-DMTK_JPEG_HW_DECODER")
			p.Target.Android.Cflags = append(p.Target.Android.Cflags, "-DMTK_JPEG_HW_REGION_RESIZER")
			p.Target.Android.Cflags = append(p.Target.Android.Cflags, "-DMTK_SKIA_MULTI_THREAD_JPEG_REGION")
			p.Target.Android.Cflags = append(p.Target.Android.Cflags, "-DSK_JPEG_INDEX_SUPPORTED_MTK")
			p.Target.Android.Cflags = append(p.Target.Android.Cflags, "-DMTK_IMAGE_ENABLE_PQ_FOR_JPEG")
			
			includeDirs = append(includeDirs, "system/core/libion/include/")
			includeDirs = append(includeDirs, "system/core/include/utils/")
			includeDirs = append(includeDirs, "vendor/mediatek/proprietary/external/libion_mtk/include/")
			includeDirs = append(includeDirs, "device/mediatek/common/kernel-headers/")
//			includeDirs = append(includeDirs, "vendor/mediatek/proprietary/external/mtkjpeg/include/")


			legacysharedlibs = append(legacysharedlibs, "libion")
			legacysharedlibs = append(legacysharedlibs, "libmhalImageCodec")
			legacysharedlibs = append(legacysharedlibs, "libdpframework")
			legacysharedlibs = append(legacysharedlibs, "libion_mtk")
		}
	} 
	
	if v, found := featureValue["TARGET_BOARD_PLATFORM"]; found {
		if v == "mt6757" { 
			p.Target.Android.Cflags = append(p.Target.Android.Cflags, "-DMTK_SKIA_USE_ION")
		}
	} 
	
	p.Target.Android.Include_dirs = includeDirs
	
	p.Target.Android.Legacy_shared_libs = legacysharedlibs
	
	ctx.AppendProperties(p)
}
func init() {
	android.RegisterModuleType("mtk_jpeg_enhance_defaults", mtkJpegEnhanceDefaultsFactory)
}
 
func mtkJpegEnhanceDefaultsFactory() (blueprint.Module, []interface{}) {
	module, props := cc.DefaultsFactory()
	android.AddLoadHook(module, mtkJpegEnhanceDefaults)
	return module, props
}