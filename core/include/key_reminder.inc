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
* $Id: key_reminder.inc,v 1.8.10.1 2012/05/29 03:45:44 akarelia Exp $
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
* @version $Revision: 1.8.10.1 $
* @package MySource_Matrix
*/
?>
<html>
	<head>
		<title><?php echo SQ_SYSTEM_LONG_NAME ?> - Invalid Warranty Key</title>
		<link rel="stylesheet" href="<?php echo sq_web_path('lib').'/web/css/key_reminder.css' ?>" type="text/css" />
	</head>
	<body>
		<form method="post" action="<?php echo htmlentities($return_url, ENT_COMPAT, SQ_CONF_DEFAULT_CHARACTER_SET); ?>">
		<div class="key-reminder">
			<h1><?php echo sq_get_icon(sq_web_path('lib').'/web/images/icons/header/system_config.png', 20, 20, 'Key icon', '', 'align="absmiddle"') ?> Invalid Warranty Key</h1>
			<div class="key-reminder-body">
			<p class="key-reminder-warning">YOUR WARRANTY KEY IS INVALID CONSEQUENTLY YOUR SYSTEM IS CURRENTLY NOT WARRANTED.</p>

			<p>Squiz Matrix is an open source product with the benefit of full commercial support provided through Squiz Service Level Agreements. A valid <em>Warranty Key</em> is a means to alert administrators of the presence or absence of a current Squiz Service Level agreement.</p>

			<p>The <em>Warranty Key</em> is a code provided by Squiz that contains encoded information about when your system warranty will expire and a portion of your <em>Install Key</em>.  It is used by Squiz Support to provide you appropriate levels of support and warranty. It does not include any personal or other identifying information.</p>

			<p class="key-reminder-info"><?php echo sq_get_icon(sq_web_path('lib').'/web/images/icons/info.png', 16, 16, 'Info icon', '', 'align="absmiddle"') ?> NB: All keys provided to Squiz are used in accordance with Squiz's <a href="http://matrix.squiz.net/privacy-policy">Privacy Policy</a>.</p>

			<h2>Obtaining a Warranty Key</h2>

			<p>To obtain a <em>Warranty Key</em> from Squiz, you need to obtain the <em>Install Key</em> for this system. You can find the install key from the <strong>System Configuration</strong> screen.</p>

			<p>Once you have obtained the <em>Install Key</em>, you will need to send it to Squiz (contact details below) so that they may provide you with a <em>Warranty Key</em>. Once you have received a <em>Warranty Key</em>, you enter this on the <strong>System Configuration</strong> screen, next to where you obtained your <em>Install Key</em>.</p>

			<h2>Contacting Squiz</h2>

			<p>For assistance or to submit your <em>Install Key</em>, contact Squiz Support on 13000 SQUIZ (+61 1300 077 849) or email warrantykey@squiz.net.</p>

			<h2>Continue to <?php echo (!SQ_IN_LIMBO ? 'Administration' : 'Simple Edit') ?> Interface</h2>

			<p>Click the <strong>Continue</strong> button below to proceed to your backend interface. This message will not appear again until this session concludes.</p>

			<input type="submit" value="Continue">
			</div>
			<div class="key-reminder-footer"><a href="http://matrix.squiz.net/" target="_blank"><?php echo SQ_SYSTEM_LONG_NAME ?></a></div>
		</div>
		</form>
	</body>
</html>
