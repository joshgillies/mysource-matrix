<?php
/**
* Copyright (c) 2003 - Squiz Pty Ltd
*
* $Id: blank.php,v 1.6 2003/10/03 00:03:25 brobertson Exp $
* $Name: not supported by cvs2svn $
*/

/**
* Blank Pop-Up
*
* Purpose
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix_Packages
* @subpackage cms
*/


header("Cache-Control: no-store, no-cache, must-revalidate, post-check=0, pre-check=0");
header("Pragma: no-cache");
header("Expires: ". gmdate("D, d M Y H:i:s",time()-3600) . " GMT");

require(dirname(__FILE__)."/header.php");
?> 
<script language="JavaScript" type="text/javascript">
	function popup_init() {
		// do nothing, just here so that we overright the popup_init() fn 
		// that may have been set by a previous pop-up
		// needed for Netscape
	}// end popup_init()
</script>
<?php 

require(dirname(__FILE__)."/footer.php"); 

?>