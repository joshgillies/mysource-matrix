<?php
/**
* Absolute Last Resort Backup Login Form
* Used only when no login design is found in the system (REALLY BAD SITUATION)
*
* Purpose
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Version$ - 1.0
* @package Resolve
*/
?>
<html>
<head>
	<title>Login</title>
</head>
<body marginwidth="0" marginheight="0" leftmargin="0" topmargin="0" bgcolor="#ffffff">
<form action="<?php echo $_SERVER['PHP_SELF']; ?>?SQ_ACTION=login" name="login_prompt" method="post">
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
