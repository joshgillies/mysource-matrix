<?php
/**
* +--------------------------------------------------------------------+
* | MySource 3 - MySource Matrix                                       |
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
* $Id: login.inc,v 1.11 2003/11/18 15:37:34 brobertson Exp $
* $Name: not supported by cvs2svn $
*/

/**
* Absolute Last Resort Backup Login Form
* Used only when no login design is found in the system (REALLY BAD SITUATION)
*
* Purpose
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix
*/
?>
<html>
<head>
	<title>Login</title>
</head>
<body marginwidth="0" marginheight="0" leftmargin="0" topmargin="0" bgcolor="#ffffff">
<form action="<?php echo $_SERVER['PHP_SELF']; ?>?SQ_ACTION=login&FORCE_BACKUP_LOGIN=1" name="login_prompt" method="post">
<input type="hidden" name="SQ_LOGIN_KEY" value="<?php echo $GLOBALS['SQ_SYSTEM']->generateLoginKey(); ?>">
<p>
Username: 
<input type="text" name="SQ_LOGIN_USERNAME" value="" size="10" maxlength="64" onfocus="javascript: this.select();">
</p>
<p>
Password: 
<input type="password" name="SQ_LOGIN_PASSWORD" value="" size="10" maxlength="64" onfocus="javascript: this.select();">
</p>
<p>
<input type="submit" value="Login">
</p>
</form>
</body>
</html>
