<?php
define('SQ_SYSTEM_ROOT',		dirname(dirname(dirname(dirname(dirname(dirname(realpath(__FILE__))))))));
define('SQ_INCLUDE_PATH',		SQ_SYSTEM_ROOT.'/core/include');
define('SQ_CORE_PACKAGE_PATH',	SQ_SYSTEM_ROOT.'/core/assets');
define('SQ_ATTRIBUTES_PATH',	SQ_SYSTEM_ROOT.'/core/attributes');
define('SQ_LIB_PATH',			SQ_SYSTEM_ROOT.'/core/lib');
define('SQ_DATA_PATH',			SQ_SYSTEM_ROOT.'/data');
define('SQ_CACHE_PATH',			SQ_SYSTEM_ROOT.'/cache');
define('SQ_PACKAGES_PATH',		SQ_SYSTEM_ROOT.'/packages');
define('SQ_WEB_PATH',			SQ_SYSTEM_ROOT.'/core/web');
define('SQ_FUDGE_PATH',			SQ_SYSTEM_ROOT.'/fudge');
define('SQ_TEMP_PATH',			SQ_SYSTEM_ROOT.'/data/temp');
include_once(SQ_DATA_PATH.'/private/conf/main.inc');
include_once(SQ_INCLUDE_PATH.'/general.inc');
$finfo = stat(realpath(__FILE__));
$mod_ts = $finfo['mtime'];
header('Last-Modified: '.date('r', $mod_ts));
header('Content-Type: text/css');
?>

#sq_message_body, #inbox_container, #sq_message_body_disabled {
	width: 95%;
	height: 250px;
	margin: 5px 0px;
	border: 1px solid black;
	white-space: pre;
	overflow: auto;
	padding: 1ex;
	font-family: "Lucida Grande", "Lucida Console", "Courier New", monospace;
}
#sq_message_body_disabled {
	line-height: 250px;
	background: #CCC;
	text-align: center;
}
#inbox_container {
	white-space: normal;
	padding-top: 0;
}
#inbox_table {
	width: 100%;
	margin-left: -1px;
}
#selected_row td {
	background: #C9F;
}
.sq-backend-table-header img, .sq-backend-table-header input {
	float: left;
}
.sq-backend-table-header img {
	margin: 0 3px;
}

.read {
	background: url(<?php echo sq_web_path('lib'); ?>/web/images/icons/internal_message/message_read.png);
	background: expression('none');
	filter: progid:DXImageTransform.Microsoft.AlphaImageLoader(src='<?php echo sq_web_path('lib'); ?>/web/images/icons/internal_message/message_read.png', sizingMethod='crop');
	height: 16px;
	width: 16px;
	cursor: pointer;
}
.unread {
	background: url(<?php echo sq_web_path('lib'); ?>/web/images/icons/internal_message/message_unread.png);
	background: expression('none');
	filter: progid:DXImageTransform.Microsoft.AlphaImageLoader(src='<?php echo sq_web_path('lib'); ?>/web/images/icons/internal_message/message_unread.png', sizingMethod='crop');
	height: 16px;
	width: 16px;
	cursor: pointer;
}
.trash {
	background: url(<?php echo sq_web_path('lib'); ?>/web/images/icons/internal_message/trash.png);
	background: expression('none');
	filter: progid:DXImageTransform.Microsoft.AlphaImageLoader(src='<?php echo sq_web_path('lib'); ?>/web/images/icons/internal_message/trash.png', sizingMethod='crop');
	height: 16px;
	width: 16px;
	cursor: pointer;
}
.trash {
	background: url(<?php echo sq_web_path('lib'); ?>/web/images/icons/internal_message/trash.png);
	background: expression('none');
	filter: progid:DXImageTransform.Microsoft.AlphaImageLoader(src='<?php echo sq_web_path('lib'); ?>/web/images/icons/internal_message/trash.png', sizingMethod='crop');
	height: 16px;
	width: 16px;
	cursor: pointer;
}
.prior1, .prior2, .prior3, .prior4, .prior5 {
	height: 16px;
	width: 16px;
	float: left;
}
.prior1 {
	background: url(<?php echo sq_web_path('lib'); ?>/web/images/icons/internal_message/priority_1.png);
	background: expression('none');
	filter: progid:DXImageTransform.Microsoft.AlphaImageLoader(src='<?php echo sq_web_path('lib'); ?>/web/images/icons/internal_message/priority_1.png', sizingMethod='crop');
}
.prior2 {
	background: url(<?php echo sq_web_path('lib'); ?>/web/images/icons/internal_message/priority_2.png);
	background: expression('none');
	filter: progid:DXImageTransform.Microsoft.AlphaImageLoader(src='<?php echo sq_web_path('lib'); ?>/web/images/icons/internal_message/priority_2.png', sizingMethod='crop');
}
.prior3 {
	background: url(<?php echo sq_web_path('lib'); ?>/web/images/icons/internal_message/priority_3.png);
	background: expression('none');
	filter: progid:DXImageTransform.Microsoft.AlphaImageLoader(src='<?php echo sq_web_path('lib'); ?>/web/images/icons/internal_message/priority_3.png', sizingMethod='crop');
}
.prior4 {
	background: url(<?php echo sq_web_path('lib'); ?>/web/images/icons/internal_message/priority_4.png);
	background: expression('none');
	filter: progid:DXImageTransform.Microsoft.AlphaImageLoader(src='<?php echo sq_web_path('lib'); ?>/web/images/icons/internal_message/priority_4.png', sizingMethod='crop');
}
.prior5 {
	background: url(<?php echo sq_web_path('lib'); ?>/web/images/icons/internal_message/priority_5.png);
	background: expression('none');
	filter: progid:DXImageTransform.Microsoft.AlphaImageLoader(src='<?php echo sq_web_path('lib'); ?>/web/images/icons/internal_message/priority_5.png', sizingMethod='crop');
}

