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
* $Id: password_change.inc,v 1.3 2012/08/30 01:09:20 ewang Exp $
*
*/

/**
* Absolute Last Resort Backup Login Form
* Used only when no login design is found in the system (REALLY BAD SITUATION)
*
* Purpose
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Revision: 1.3 $
* @package MySource_Matrix
*/
?>
<html>
	<head>
		<title>Squiz Matrix Change User Password</title>
	</head>
	<body bgcolor="#FFFFFF">
		<form action="<?php echo $_SERVER['PHP_SELF']; ?>?SQ_ACTION=password_change&FORCE_BACKUP_PASSWORD_CHANGE=1" name="password_change_prompt" method="post">
			<p>
				Current Password:
				<input type="password" name="SQ_PASSWORD_CHANGE_OLD" value="" size="10" maxlength="64" onfocus="javascript: this.select();" />
			</p>
			<p>
				New Password:
				<input type="password" name="SQ_PASSWORD_CHANGE_NEW" value="" size="10" maxlength="64" onfocus="javascript: this.select();" />
			</p>
			<p>
				Confirm New Password:
				<input type="password" name="SQ_PASSWORD_CHANGE_CONFIRM" value="" size="10" maxlength="64" onfocus="javascript: this.select();" />
			</p>
			<p>
				<input type="submit" value="Change Password" />
			</p>
		</form>
	</body>
</html>
