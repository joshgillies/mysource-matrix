<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ABN 77 084 670 600                                                 |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.com.au) so we may provide|
* | you a copy.                                                        |
* +--------------------------------------------------------------------+
*
* $Id: inbox.css.php,v 1.7 2012/08/30 01:09:16 ewang Exp $
*
*/

include_once dirname(dirname(dirname(dirname(dirname(dirname(realpath(__FILE__))))))).'/core/include/init.inc';
$finfo = stat(realpath(__FILE__));
$mod_ts = $finfo['mtime'];
header('Last-Modified: '.date('r', $mod_ts));
header('Content-Type: text/css');
?>

#sq_message_body, #inbox_container {
	width:			95%;
	height:			250px;
	margin:			5px 0px;
	border:			1px solid black;
	white-space:	pre;
	overflow:		auto;
	padding:		1ex;
	font-family:	"Lucida Grande", "Lucida Console", "Courier New", monospace;
}
#inbox_container {
	white-space:	normal;
	padding-top:	0;
}
#inbox_table {
	width:			100%;
	margin-left:	-1px;
}
#selected_row td {
	background:		#C8BDCB;
}
.sq-backend-table-header img, .sq-backend-table-header input {
	float:			left;
}
.sq-backend-table-cell img, .sq-backend-table-cell input {
	float:			left;
}
.read {
	background:		url(<?php echo sq_web_path('lib'); ?>/web/images/icons/internal_message/message_read.png);
	background:		expression('none');
	filter:			progid:DXImageTransform.Microsoft.AlphaImageLoader(src='<?php echo sq_web_path('lib'); ?>/web/images/icons/internal_message/message_read.png', sizingMethod='crop');
	height:			16px;
	width:			16px;
}
.unread {
	background:		url(<?php echo sq_web_path('lib'); ?>/web/images/icons/internal_message/message_unread.png);
	background:		expression('none');
	filter:			progid:DXImageTransform.Microsoft.AlphaImageLoader(src='<?php echo sq_web_path('lib'); ?>/web/images/icons/internal_message/message_unread.png', sizingMethod='crop');
	height:			16px;
	width:			16px;
}
.trash {
	background:		url(<?php echo sq_web_path('lib'); ?>/web/images/icons/internal_message/trash.png);
	background:		expression('none');
	filter:			progid:DXImageTransform.Microsoft.AlphaImageLoader(src='<?php echo sq_web_path('lib'); ?>/web/images/icons/internal_message/trash.png', sizingMethod='crop');
	height:			16px;
	width:			16px;
}
.prior1, .prior2, .prior3, .prior4, .prior5 {
	height:			16px;
	width:			16px;
	float:			left;
}
.prior1 {
	background:		url(<?php echo sq_web_path('lib'); ?>/web/images/icons/internal_message/priority_1.png);
	background:		expression('none');
	filter:			progid:DXImageTransform.Microsoft.AlphaImageLoader(src='<?php echo sq_web_path('lib'); ?>/web/images/icons/internal_message/priority_1.png', sizingMethod='crop');
}
.prior2 {
	background:		url(<?php echo sq_web_path('lib'); ?>/web/images/icons/internal_message/priority_2.png);
	background:		expression('none');
	filter:			progid:DXImageTransform.Microsoft.AlphaImageLoader(src='<?php echo sq_web_path('lib'); ?>/web/images/icons/internal_message/priority_2.png', sizingMethod='crop');
}
.prior3 {
	background:		url(<?php echo sq_web_path('lib'); ?>/web/images/icons/internal_message/priority_3.png);
	background:		expression('none');
	filter:			progid:DXImageTransform.Microsoft.AlphaImageLoader(src='<?php echo sq_web_path('lib'); ?>/web/images/icons/internal_message/priority_3.png', sizingMethod='crop');
}
.prior4 {
	background:		url(<?php echo sq_web_path('lib'); ?>/web/images/icons/internal_message/priority_4.png);
	background:		expression('none');
	filter:			progid:DXImageTransform.Microsoft.AlphaImageLoader(src='<?php echo sq_web_path('lib'); ?>/web/images/icons/internal_message/priority_4.png', sizingMethod='crop');
}
.prior5 {
	background:		url(<?php echo sq_web_path('lib'); ?>/web/images/icons/internal_message/priority_5.png);
	background:		expression('none');
	filter:			progid:DXImageTransform.Microsoft.AlphaImageLoader(src='<?php echo sq_web_path('lib'); ?>/web/images/icons/internal_message/priority_5.png', sizingMethod='crop');
}

