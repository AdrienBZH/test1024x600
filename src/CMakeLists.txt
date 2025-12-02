SET(SOURCES screens/ui_Screen1.c
    screens/ui_Screen2.c
    ui.c
    components/ui_comp_hook.c
    ui_helpers.c
    ui_img_manager.c
    images/ui_img_logo_loading_70_png.c
    images/ui_img_config_led_std_png.c)

add_library(ui ${SOURCES})
