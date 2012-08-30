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
* $Id: blank.php,v 1.7 2012/08/30 01:09:05 ewang Exp $
*
*/

/**
* Blank Pop-Up
*
* Purpose
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.7 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/


header('Cache-Control: no-store, no-cache, must-revalidate, post-check=0, pre-check=0');
header('Pragma: no-cache');
header('Expires: '.gmdate('D, d M Y H:i:s', time()-3600).' GMT');

require(dirname(__FILE__).'/header.php');
?>
<script language="JavaScript" type="text/javascript">
	function popup_init() {
		// do nothing, just here so that we overright the popup_init() fn
		// that may have been set by a previous pop-up
		// needed for Netscape
	}// end popup_init()
</script>
<?php

require(dirname(__FILE__).'/footer.php');

?>