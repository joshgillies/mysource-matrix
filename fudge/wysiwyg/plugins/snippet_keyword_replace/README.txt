Snippet keyword dropdown WYSIWYG plugins
    - show list of snippet keyword under selected root node based on user's permission
    - works like existing matrix keyword dropdown, will add snippet keyword to the current cursor location
    - able to enable/disable this plugin from the WYSIWYG plugin global preference screen
    - format: %globals_snippet_<snippet_id>_<snippet_name>%, name is optional
    - snippet info 'i' button will display additional help text info in a pop up based on user's permission
    - description can be edited on bodycopy container properties pop up, and description attribute
    - keywords will not work in snippet as global replacement only happens once
    - nested snippet, no read access, unknown snippet keyword will be blanked out when previewed
    - keyword order in dropdown is based on the asset map sort order
    - see am::getSnippetKeywords()
=========================
/home/cvs/mysource_matrix/core/mysource_matrix/core/include/asset_manager.inc,v  <--  core/include/asset_manager.inc
new revision: 1.583; previous revision: 1.582
/home/cvs/mysource_matrix/core/mysource_matrix/core/include/general.inc,v  <--  core/include/general.inc
new revision: 1.175; previous revision: 1.174
/home/cvs/mysource_matrix/core/mysource_matrix/core/assets/content_types/content_type_wysiwyg/content_type_wysiwyg_edit_fns.inc,v  <--  
core/assets/content_types/content_type_wysiwyg/content_type_wysiwyg_edit_fns.inc
new revision: 1.40; previous revision: 1.39
/home/cvs/fudge/wysiwyg/plugins/snippet_keyword_replace/snippet_info_popup.php,v  <--  
fudge/wysiwyg/plugins/snippet_keyword_replace/snippet_info_popup.php
initial revision: 1.1
/home/cvs/fudge/wysiwyg/plugins/snippet_keyword_replace/snippet_keyword_replace.inc,v  <--  
fudge/wysiwyg/plugins/snippet_keyword_replace/snippet_keyword_replace.inc
initial revision: 1.1


Added textarea input in the container properties pop up (div and table)
Added textarea input for description attribute in the bodycopy container details screen
%asset_attribute_description% keyword will be replaced correctly
Modified popup dimension and CSS styles

Note: PLEASE CLEAR BROWSER CACHE AFTER STEP 3
=========================
/home/cvs/mysource_matrix/core/mysource_matrix/core/assets/bodycopy/bodycopy/js/bodycopy_edit_divs.js,v  <--  
core/assets/bodycopy/bodycopy/js/bodycopy_edit_divs.js
new revision: 1.11; previous revision: 1.10
/home/cvs/mysource_matrix/core/mysource_matrix/core/assets/bodycopy/bodycopy/js/bodycopy_edit_tables.js,v  <--  
core/assets/bodycopy/bodycopy/js/bodycopy_edit_tables.js
new revision: 1.11; previous revision: 1.10
/home/cvs/mysource_matrix/core/mysource_matrix/core/assets/bodycopy/bodycopy/popups/edit_div_props.php,v  <--  
core/assets/bodycopy/bodycopy/popups/edit_div_props.php
new revision: 1.21; previous revision: 1.20
/home/cvs/mysource_matrix/core/mysource_matrix/core/assets/bodycopy/bodycopy/popups/edit_table_props.php,v  <--  
core/assets/bodycopy/bodycopy/popups/edit_table_props.php
new revision: 1.13; previous revision: 1.12
/home/cvs/mysource_matrix/core/mysource_matrix/core/assets/bodycopy/bodycopy/popups/header.php,v  <--  
core/assets/bodycopy/bodycopy/popups/header.php
new revision: 1.8; previous revision: 1.7
/home/cvs/mysource_matrix/core/mysource_matrix/core/assets/bodycopy/bodycopy_container/asset.xml,v  <--  
core/assets/bodycopy/bodycopy_container/asset.xml
new revision: 1.11; previous revision: 1.10
/home/cvs/mysource_matrix/core/mysource_matrix/core/assets/bodycopy/bodycopy_container/bodycopy_container_edit_fns.inc,v  <--  
core/assets/bodycopy/bodycopy_container/bodycopy_container_edit_fns.inc
new revision: 1.55; previous revision: 1.54
/home/cvs/mysource_matrix/core/mysource_matrix/core/assets/bodycopy/bodycopy_container/bodycopy_container_management.inc,v  <--  
core/assets/bodycopy/bodycopy_container/bodycopy_container_management.inc
new revision: 1.11; previous revision: 1.10
/home/cvs/mysource_matrix/core/mysource_matrix/core/assets/bodycopy/bodycopy_container/edit_interface_screen_details.xml,v  <--  
core/assets/bodycopy/bodycopy_container/edit_interface_screen_details.xml
new revision: 1.7; previous revision: 1.6
/home/cvs/mysource_matrix/core/mysource_matrix/core/assets/bodycopy/bodycopy_container/locale/en/lang_screen_details.xml,v  <--  
core/assets/bodycopy/bodycopy_container/locale/en/lang_screen_details.xml
new revision: 1.3; previous revision: 1.2
