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
*
* $Id: key_reminder.inc,v 1.1.2.2 2007/01/08 03:49:51 lwright Exp $
*
*/

/**
* Warranty Expiry Reminder Message
* Static message used when there is a warranty key provided by Squiz - but it
* has expired
*
* Purpose
*
* @author  Luke Wright <lwright@squiz.net>
* @version $Revision: 1.1.2.2 $
* @package MySource_Matrix
*/
?>
<html>
	<head>
		<title><?php echo SQ_SYSTEM_LONG_NAME ?> - Warranty Expiry Message</title>
		<link rel="stylesheet" href="<?php echo sq_web_path('lib').'/web/css/key_reminder.css' ?>" type="text/css" />
	</head>
	<body>
		<?php
			$date_expired = substr(base64_decode(SQ_LICENCE_WARRANTY_KEY), 4);
			$date_expired = date('l, jS F Y', strtotime($date_expired));
		?>
		<form method="get" action="<?php echo $return_url ?>">
		<div class="key-reminder">
			<h1><?php echo sq_get_icon(sq_web_path('lib').'/web/images/icons/header/system_config.png', 20, 20, 'Key icon', '', 'align="absmiddle"') ?> Warranty Expired</h2>
			<div class="key-reminder-body">
			<p>Our records indicate that your warranty for this system expired on <strong><?php echo $date_expired; ?></strong>.</p>

			<h2>Continue to <?php echo (!SQ_IN_LIMBO ? 'Administration' : 'Simple Edit') ?> Interface</h3>

			<p>Click the <strong>Continue</strong> button below to proceed to your backend interface. This message will not appear again until this session concludes.</p>

			<input type="submit" value="Continue">
			</div>
			<div class="key-reminder-footer"><a href="http://matrix.squiz.net/" target="_blank"><?php echo SQ_SYSTEM_LONG_NAME ?></a></div>
		</div>
		</form>
	</body>
</html>
