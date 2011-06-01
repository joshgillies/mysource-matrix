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
* $Id: embed_youtube_frames.php,v 1.1 2008/10/31 03:46:51 bpearson Exp $
*
*/
?>

<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Frameset//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-frameset.dtd">

<html>
<head>
<title>Embed YouTube</title>
</head>
<frameset rows="30,*" frameborder="0" border="0">
	<frame src="embed_youtube_title.php" name="sq_wysiwyg_popup_title" scrolling="no" marginwidth="0" marginheight="0" noresize="noresize" />
	<frame src="embed_youtube.php?<?php echo $_SERVER['QUERY_STRING'] ?>" name="sq_wysiwyg_popup_main" scrolling="no" marginwidth="0" marginheight="0" noresize="noresize" />
</frameset>
</html>
