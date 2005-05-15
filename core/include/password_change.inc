<?php
/**
* +--------------------------------------------------------------------+
* | Squiz.net Open Source Licence                                      |
* +--------------------------------------------------------------------+
* | Copyright (c), 2003 Squiz Pty Ltd (ABN 77 084 670 600).            |
* +--------------------------------------------------------------------+
* | This source file may be used subject to, and only in accordance    |
* | with, the Squiz Open Source Licence Agreement found at             |
* | http://www.squiz.net/licence.                                      |
* | Make sure you have read and accept the terms of that licence,      |
* | including its limitations of liability and disclaimers, before     |
* | using this software in any way. Your use of this software is       |
* | deemed to constitute agreement to be bound by that licence. If you |
* | modify, adapt or enhance this software, you agree to assign your   |
* | intellectual property rights in the modification, adaptation and   |
* | enhancement to Squiz Pty Ltd for use and distribution under that   |
* | licence.                                                           |
* +--------------------------------------------------------------------+
*
* $Id: password_change.inc,v 1.1 2005/05/15 23:45:36 ndvries Exp $
*
*/

/**
* Absolute Last Resort Backup Login Form
* Used only when no login design is found in the system (REALLY BAD SITUATION)
*
* Purpose
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Revision: 1.1 $
* @package MySource_Matrix
*/
?>
<html>
	<head>
		<title>MySource Matrix Change User Password</title>
	</head>
	<body bgcolor="#FFFFFF">
		<form action="<?php echo $_SERVER['PHP_SELF']; ?>?SQ_ACTION=password_change&FORCE_BACKUP_PASSWORD_CHANGE=1" name="password_change_prompt" method="post">
			<p>
				Old Password:
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
