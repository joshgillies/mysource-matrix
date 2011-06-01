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
* $Id: blank.php,v 1.6 2006/12/05 05:34:15 emcdonald Exp $
*
*/

/**
* Blank Pop-Up
*
* Purpose
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.6 $
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