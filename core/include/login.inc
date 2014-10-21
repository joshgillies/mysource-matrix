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
*
* $Id: login.inc,v 1.20 2012/08/30 01:09:19 ewang Exp $
*
*/

/**
* Absolute Last Resort Backup Login Form
* Used only when no login design is found in the system (REALLY BAD SITUATION)
*
* Purpose
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Revision: 1.20 $
* @package MySource_Matrix
*/
?>
<html>
	<head>
		<title>MySource Matrix Login</title>
	</head>
	<body bgcolor="#FFFFFF">
		<form action="<?php echo $_SERVER['PHP_SELF']; ?>?SQ_ACTION=login&FORCE_BACKUP_LOGIN=1" name="login_prompt" method="post">
			<input type="hidden" name="SQ_LOGIN_KEY" value="<?php echo $GLOBALS['SQ_SYSTEM']->generateLoginKey(); ?>" />
			<p>
				<?php echo translate('Username'); ?>:

				<input type="text" name="SQ_LOGIN_USERNAME" value="" size="10" maxlength="64" onfocus="javascript: this.select();" />
			</p>
			<p>
				<?php echo translate('Password'); ?>:

				<input type="password" name="SQ_LOGIN_PASSWORD" value="" size="10" maxlength="64" onfocus="javascript: this.select();" />
			</p>
			<p>
				<input type="submit" value="Login" />
			</p>
		</form>
	</body>
</html>
