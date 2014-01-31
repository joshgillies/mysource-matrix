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
* $Id: insert_link_frames.php,v 1.3 2012/08/30 00:56:52 ewang Exp $
*
*/
?>

<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Frameset//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-frameset.dtd">

<html>
<head>
    <title>Insert Link</title>
</head>
<frameset rows="30,*" frameborder="0" border="0">
	<frameset cols="*, 500"  frameborder="0" border="0">
		<frame src="insert_link_title.php" name="sq_wysiwyg_popup_title" scrolling="no" marginwidth="0" marginheight="0" noresize="noresize" />
		<frame src="insert_link_search.php" name="sq_wysiwyg_popup_search" scrolling="no" marginwidth="0" marginheight="0" noresize="noresize" />
	</frameset>
    <frame src="insert_link.php?<?php echo htmlspecialchars($_SERVER['QUERY_STRING'], ENT_QUOTES); ?>" name="sq_wysiwyg_popup_main" scrolling="no" marginwidth="0" marginheight="0" noresize="noresize" />    
</frameset>
</html>
