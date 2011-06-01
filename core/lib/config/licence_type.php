<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ACN 084 670 600                                                    |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.net) so we may provide   |
* | you a copy.                                                        |
* +--------------------------------------------------------------------+
*
* $Id: licence_type.php,v 1.2 2008/11/19 00:58:43 hnguyen Exp $
*
*/

/* 
* This file display the licence type for the system it lies in 
*
* @author	Huan Nguyen <hnguyen@squiz.net>
* @version  $Revision: 1.0
*/

// Make sure this file is always in MATRIX_ROOT/core/lib/config/ folder
if (file_exists(dirname(dirname(dirname(dirname(__FILE__))))."/data/private/conf/licence.inc")) {
	// Make sure no information get out of the licence file
	ob_start();
		require_once(dirname(dirname(dirname(dirname(__FILE__))))."/data/private/conf/licence.inc");
	ob_end_clean();

	if (defined('SQ_LICENCE_TYPE')) {
		echo(SQ_LICENCE_TYPE);
	} else {
		echo 'Unknown Licence Type';
	}//end else
} else {
	echo 'Licence File has not been generated';
}//end if

?>
