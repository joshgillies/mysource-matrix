<?php
/**
* Copyright (c) 2003 - Squiz Pty Ltd
*
* $Id: login.inc,v 1.10 2003/09/26 05:26:31 brobertson Exp $
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
