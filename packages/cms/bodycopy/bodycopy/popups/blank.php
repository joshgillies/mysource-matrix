<?php
/**
* Blank Pop-Up
*
* Purpose
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Version$ - 1.0
* @package Resolve_Packages
* @subpackage cms
*/


header("Cache-Control: no-store, no-cache, must-revalidate, post-check=0, pre-check=0");
header("Pragma: no-cache");
header("Expires: ". gmdate("D, d M Y H:i:s",time()-3600) . " GMT");

include(dirname(__FILE__)."/header.php");
?> 
<script language="JavaScript">
	function popup_init() {
		// do nothing, just here so that we overright the popup_init() fn 
		// that may have been set by a previous pop-up
		// needed for Netscape
	}// end popup_init()
</script>
<?php include(dirname(__FILE__)."/footer.php"); ?>