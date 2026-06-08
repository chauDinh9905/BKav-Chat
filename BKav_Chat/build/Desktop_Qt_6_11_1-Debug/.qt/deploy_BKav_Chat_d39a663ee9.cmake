include("/home/chau-dinh/Documents/intern/projects_for_desktop/BKav_Chat/build/Desktop_Qt_6_11_1-Debug/.qt/QtDeploySupport.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/BKav_Chat-plugins.cmake" OPTIONAL)
set(__QT_DEPLOY_I18N_CATALOGS "qtbase")

qt6_deploy_runtime_dependencies(
    EXECUTABLE "/home/chau-dinh/Documents/intern/projects_for_desktop/BKav_Chat/build/Desktop_Qt_6_11_1-Debug/BKav_Chat"
    GENERATE_QT_CONF
)
