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
* $Id: login.inc,v 1.19 2006/12/05 05:36:14 bcaldwell Exp $
*
*/

/**
* Absolute Last Resort Backup Login Form
* Used only when no login design is found in the system (REALLY BAD SITUATION)
*
* Purpose
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Revision: 1.19 $
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
				<?php echo translate('username'); ?>:
				<input type="text" name="SQ_LOGIN_USERNAME" value="" size="10" maxlength="64" onfocus="javascript: this.select();" />
			</p>
			<p>
				<?php echo translate('password'); ?>:
				<input type="password" name="SQ_LOGIN_PASSWORD" value="" size="10" maxlength="64" onfocus="javascript: this.select();" />
			</p>
			<p>
				<input type="submit" value="Login" />
			</p>
		</form>
	</body>
</html>
