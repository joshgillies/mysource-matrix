<?php

function print_login($heading, $message) { 

	$referer = !isset($_POST['SQ_LOGIN_REFERER']) ? (empty($_SERVER['HTTP_REFERER']) ? '' : $_SERVER['HTTP_REFERER']) : $_POST['SQ_LOGIN_REFERER'];

	header("HTTP/1.0 403 Forbidden");

	$qry_string = '?SQ_ACTION=login'.preg_replace('/SQ_ACTION=[^\&]+/', '', $_SERVER['QUERY_STRING']);

	?>
	<html>
	<head>
		<title><?php echo $heading ?></title>
		<link rel="stylesheet" type="text/css" href="<?php echo SQ_WWW_LIB_DIR; ?>/web/css/edit.css">
	</head>
	<body marginwidth="0" marginheight="0" leftmargin="0" topmargin="0" bgcolor="#ffffff" onLoad="javascript: document.login_prompt.elements['SQ_LOGIN_USERNAME'].focus();">
	<form action="<?php echo $_SERVER['PHP_SELF'].$qry_string ; ?>" name="login_prompt" method="post" onSubmit="javascript: return (this.elements['SQ_LOGIN_USERNAME'].value.length > 0 && this.elements['SQ_LOGIN_PASSWORD'].value.length > 1);">
	<input type="hidden" name="SQ_LOGIN_KEY"      value="<?php echo $GLOBALS['SQ_RESOLVE']->generateLoginKey(); ?>">
	<input type="hidden" name="SQ_LOGIN_REFERER"  value="<?php echo $referer; ?>">
	<?php

	/*
	# Take all the post vars and whack em in the form - important ones will get overwitten
	unset($_POST['mysource_login']);
	unset($_POST['mysource_password']);
	unset($_POST['mysource_login_key']);
	unset($_POST['mysource_login_referer']);
	unset($_POST['mysource_session_action']);
	echo var_to_hidden_field("",$_POST,1);
	*/
	?>
	<table height="100%" width="100%"><tr><td align="center" valign="middle">

	<table width="400" cellpadding="1" cellspacing="0" border="0" align="center">
		<tr>
			<td class="sq-backend-heading">
				<img src="<?php echo SQ_WWW_LIB_DIR; ?>/web/images/icons/swish.gif" width="20" height="20" border="0" align="left" alt="<?php echo $heading; ?>" />
				<?php echo $heading; ?>
			</td>
		</tr>
		<tr>
			<td>
				<table cellpadding="3" cellspacing="0" border="0" width="100%">
				<?php if ($msgs = $GLOBALS['SQ_RESOLVE']->messages()) { ?>
					<tr>
						<td valign="top" class="sq-backend-warning">
							<?php echo implode("<br/>", $msgs); ?>
						</td>
					</tr>
				<?php } ?>
					<tr>
						<td valign="top" class="sq-backend-sectionheading" style="font-weight: bold;">
					<?php if ($GLOBALS['SQ_RESOLVE']->currentUserId()) { ?>
							Currently logged in as: <?php echo $GLOBALS['SQ_RESOLVE']->user->name; ?>
					<?php } else { ?>
							Not currently logged in.
					<?php } ?>
						</td>
					</tr>
					<tr>
						<td valign="top" class="sq-backend-data">
							<br/>
							<?php echo $message ; ?>
							<br/><br/>
						</td>
					</tr>
					<tr>
						<td valign="top" class="sq-backend-field">
							<table width="100%" border="0" cellspacing="0" cellpadding="3">
								<tr>
									<td rowspan="3" valign="bottom" class="sq-backend-field">
										<input type="button" class="sq-backend-data" value="&lt;- Back" onclick="javascript: <?php echo (($referer) ? 'window.location=\''.addslashes($referer).'\'' : 'alert(\'Unable to derive where you came from\');'); ?>">&nbsp;
									</td>
									<td nowrap valign="top" class="sq-backend-field">
										Login: 
									</td>
									<td width="100%" valign="top" class="sq-backend-data">
										<input type="text" name="SQ_LOGIN_USERNAME" value="" size="10" maxlength="64" class="sq-backend-data" onfocus="javascript: this.select();">
									</td>
									<td rowspan="2" valign="bottom" align="right" class="sq-backend-field">
										&nbsp;
									</td>
								</tr>
								<tr>
									<td nowrap valign="top" class="sq-backend-field">
										Password: 
									</td>
									<td width="100%" valign="top" class="sq-backend-data">
										<input type="password" name="SQ_LOGIN_PASSWORD" value="" size="10" maxlength="64"  class="sq-backend-data" onfocus="javascript: this.select();">
									</td>
								</tr>
								<tr>
									<td class="sq-backend-field">
										&nbsp;
									</td>
									<td class="sq-backend-field">
										&nbsp;
									</td>
									<td valign="bottom" align="right" class="sq-backend-field">
										&nbsp;<input type="submit" class="sq-backend-data" value="Login -&gt;">
									</td>
								</tr>
							</table>
						</td>
					</tr>
				</table>
			</td>
		</tr>
	</table>

	</td></tr></table>
	</form>
	</body>
	</html>
<?php

}// end print_login()

?>