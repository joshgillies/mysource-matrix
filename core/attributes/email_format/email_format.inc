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
* $Id: email_format.inc,v 1.8 2004/09/08 04:17:47 gsherwood Exp $
* $Name: not supported by cvs2svn $
*/


require_once SQ_ATTRIBUTES_PATH.'/wysiwyg/wysiwyg.inc';

/**
* Email Format Asset Attribute
*
*
*
*
* <!-- Sample Edit Interface XML Node -->
*    <email_format />
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix
* @subpackage attributes
*/
class Asset_Attribute_Email_Format extends Asset_Attribute_WYSIWYG
{


	/**
	* Constructor
	*
	* @param int	$attributeid	the attribute id to be loaded
	* @param mixed	$value			the current value for the element
	*
	*/
	function Asset_Attribute_Email_Format($attributeid=0, $value=null)
	{
		$this->Asset_Attribute_WYSIWYG($attributeid, $value);

	}//end constructor


	/**
	* Print the interface for filling in a value
	*
	* @param string		$prefix		prefix for the form elements
	* @param boolean	$read_only	are we just printing the value
	*
	* @access public
	*/
	function paint($prefix, $read_only=false)
	{
		$current_value = @unserialize($this->value);
		if (!is_array($current_value))             $current_value = Array();
		if (!isset($current_value['to']))          $current_value['to'] = Array();
		if (!isset($current_value['from']))        $current_value['from'] = '';
		if (!isset($current_value['subject']))     $current_value['subject'] = '';
		if (!isset($current_value['html_format'])) $current_value['html_format'] = '';
		if (!isset($current_value['text_format'])) $current_value['text_format'] = '';
		
		if ($read_only) {
			?>
			<script language="Javascript">
				function emailFormatSwitchEditingMode(editor_name) {
					var textDiv = document.getElementById(editor_name + "_text_format");
					var htmlDiv = document.getElementById(editor_name + "_html_format");
					var formatSpan = document.getElementById(editor_name + "_format_mode");
				
					if (htmlDiv.style.display == "none") {

						htmlDiv.style.display = "";
						textDiv.style.display = "none";
						formatSpan.innerHTML = 'HTML Email Version';
	
					} else {
						
						textDiv.style.display = "";
						htmlDiv.style.display = "none";
						formatSpan.innerHTML = 'Text Email Version';
					}
					
				}//end emailFormatSwitchEditingMode()
			</script>
			<table border="0">
				<tr>
					<td align="right" valign="top"><b>To</b></td>
					<td valign="top">
						<?php
						foreach ($current_value['to'] as $email) {
							echo htmlspecialchars($email).'<br />';
						}
						?>
					</td>
				</tr>
				<tr>
					<td align="right" valign="top"><b>From</b></td>
					<td valign="top"><?php echo htmlspecialchars($current_value['from']); ?></td>
				</tr>
				<tr>
					<td align="right" valign="top"><b>Subject</b></td>
					<td valign="top"><?php echo $current_value['subject']; ?></td>
				</tr>
				<tr>
					<td align="right" valign="top"><b>Body</b></td>
					<td>
						<table style="width:100%;">
							<tr>
								<td align="left" width="100%" style="border: 1px dashed #C0C0C0;">
									<table>
										<tr>
											<td width="100%" align="left">
												<a href="Javascript: emailFormatSwitchEditingMode('<?php echo $prefix; ?>');"><script language="JavaScript" type="text/javascript">sq_print_icon("<?php echo sq_web_path('lib')?>/web/images/icons/edit_mode.png", "16", "16", "");</script></a>
											</td>
											<td align="right" nowrap="nowrap">
												<span id="<?php echo $prefix; ?>_format_mode">Text Email Version</span>
											</td>
										</tr>
									</table>
								</td>
							</tr>
							<tr>
								<td style="border-left: 1px dashed #C0C0C0; border-right: 1px dashed #C0C0C0; border-bottom: 1px dashed #C0C0C0;">
									<div style="display:none;" id="<?php echo $prefix?>_html_format">
										<?php echo $current_value['html_format']; ?>
									</div>
									<div id="<?php echo $prefix?>_text_format">
										<?php echo nl2br($current_value['text_format']); ?>
									</div>
								</td>
							</tr>
						</table>
					</td>
				</tr>
			</table>
			<?php
			
			return;
			
		}//end if read only


		$wysiwyg = $this->_createEditor($prefix);
		$wysiwyg->set_contents($current_value['html_format']);
		
		$current_to_string = '';
		$current_to = '';
		foreach ($current_value['to'] as $email) {
			$current_to_string .= '[ <a href="#" onclick="Javascript: emailFormatDeleteAddress(\''.$email.'\', \''.$prefix.'\', \'to\'); return false;">del</a> ] '.htmlspecialchars($email).'<br>';
			$current_to .= $email.';';
		}

		?>
		<script language="Javascript">

			function emailFormatAddAddress(editor_name, type) {
				var value = document.getElementById(editor_name + "_" + type);
				var box = document.getElementById(editor_name + "_new_" + type);
				var div = document.getElementById(editor_name + "_current_" + type);
				
				if (box.value == "") return;

				div.innerHTML += '[ <a href="#" onclick="Javascript: emailFormatDeleteAddress(\'' + box.value + '\', \'' + editor_name + '\', \'' + type + '\'); return false;">del</a> ] ' + box.value.replace('<', '&lt;').replace('>', '&gt;') + '<br>';
				value.value   += box.value + ";";
				
				box.value = "";
				box.focus();
				
			}//end emailFormatAddAddress()


			function emailFormatDeleteAddress(address, editor_name, type) {
				var value = document.getElementById(editor_name + "_" + type);
				var div = document.getElementById(editor_name + "_current_" + type);
				
				var current_emails = div.innerHTML;

				lastIndex = i = 0;
				arrIndex = 0;
				email_array = new Array();
				while(-1 != (i = current_emails.toLowerCase().indexOf("<br>", lastIndex))) {
					email_array[arrIndex] = current_emails.substring(lastIndex, i).replace('&lt;', '<').replace('&gt;', '>');
					arrIndex++;
					lastIndex = i+4;
				}
				
				var new_div_content = '';
				var new_value_content = '';

				for (var i = 0; i < email_array.length; i++) {
					var email = email_array[i];
					re = new RegExp("\\\[.*\\\] (.*)", "g");
					email_addr = email.replace(re, "\$1");

					if (email_addr != address && email_addr != "") {
						new_div_content += email + "<br>";
						new_value_content += email_addr + ";";
					}
				}
				
				div.innerHTML = new_div_content;
				value.value = new_value_content;

			}//end emailFormatDeleteAddress()
			
			var initialisedEmailEditors = new Array();
			function emailFormatSwitchEditingMode(editor_name) {
				var editor = eval('editor_' + editor_name);

				var textDiv = document.getElementById(editor_name + "_text_body_div");
				var htmlDiv = document.getElementById(editor_name + "_html_body_div");
				var formatSpan = document.getElementById(editor_name + "_format_mode");
			
				if (htmlDiv.style.display == "none") {
					var setDesignMode = true;
			
					// initilise the wysiwg if this is the first time
					// it is being shown - skip this otherwise
					if (initialisedEmailEditors[editor._uniqueID] == null) {
						initialisedEmailEditors[editor._uniqueID] = true;
						editor.generate();
						setDesignMode = false;
					} else if (editor._initialised != true) {
						return;
					}
			
					htmlDiv.style.display = "";
					
					// if we are using an iframe for this editor, we set its designMode property if we need to
					if (editor._iframe) {
						editor._iframe.style.width = editor.config.width;
						if (setDesignMode && HTMLArea.is_gecko) { editor._iframe.contentWindow.document.designMode = "on"; }
						editor._iframe.style.height = editor.config.height;
					}

					textDiv.style.display = "none";
					formatSpan.innerHTML = 'HTML Email Version';

				} else if (editor._initialised == true) {
					
					textDiv.style.display = "";
					htmlDiv.style.display = "none";
					formatSpan.innerHTML = 'Text Email Version';
				}
				
			}//end emailFormatSwitchEditingMode()

		</script>
		<table border="0">
			<tr>
				<td align="right" valign="top"><b>To</b></td>
				<td valign="top">
					<?php text_box($prefix.'_new_to', '', 50); ?>
					[ <a href="#" onclick="Javascript: emailFormatAddAddress('<?php echo $prefix ?>', 'to'); return false;">add</a> ]<br/>
					<div id="<?php echo $prefix; ?>_current_to"><?php echo $current_to_string; ?></div>
					<?php hidden_field($prefix.'_to', $current_to); ?>
				</td>
			</tr>
			<tr>
				<td align="right" valign="top"><b>From</b></td>
				<td valign="top"><?php text_box($prefix.'_from', $current_value['from'], 50); ?></td>
			</tr>
			<tr>
				<td align="right" valign="top"><b>Subject</b></td>
				<td valign="top"><?php text_box($prefix.'_subject', $current_value['subject'], 80); ?></td>
			</tr>
			<tr>
				<td align="right" valign="top"><b>Body</b></td>
				<td>
					<table style="width:100%;">
						<tr>
							<td align="left" width="100%" style="border: 1px dashed #C0C0C0;">
								<table>
									<tr>
										<td width="100%" align="left">
											<a href="Javascript: emailFormatSwitchEditingMode('<?php echo $prefix; ?>');"><script language="JavaScript" type="text/javascript">sq_print_icon("<?php echo sq_web_path('lib')?>/web/images/icons/edit_mode.png", "16", "16", "");</script></a>
										</td>
										<td align="right" nowrap="nowrap">
											<span id="<?php echo $prefix; ?>_format_mode">Text Email Version</span>
										</td>
									</tr>
								</table>
							</td>
						</tr>
						<tr>
							<td style="border-left: 1px dashed #C0C0C0; border-right: 1px dashed #C0C0C0; border-bottom: 1px dashed #C0C0C0;">
								<div style="display:none;" id="<?php echo $prefix?>_html_body_div">
									<?php echo $wysiwyg->paint()?>
								</div>
								<div id="<?php echo $prefix?>_text_body_div">
									<?php text_area($prefix.'_text_body', $current_value['text_format'], 105, 15); ?>
								</div>
							</td>
						</tr>
					</table>
				</td>
			</tr>
		</table>
		<?php

	}//end paint()


	/**
	* Process the interface for filling in a value
	*
	* @param string	prefix	prefix for the form elements
	*
	* @access public
	*/
	function process($prefix)
	{
		if (!isset($_POST[$prefix.'_to']) &&
			!isset($_POST[$prefix.'_from']) &&
			!isset($_POST[$prefix.'_subject']) &&
			!isset($_POST[$prefix.'_text_body'])) {
			return false;
		}

		$value = Array();

		$wysiwyg = &$this->_createEditor($prefix);
		$value['html_format'] = $wysiwyg->process();;
		$value['text_format'] = (isset($_POST[$prefix.'_text_body'])) ? $_POST[$prefix.'_text_body'] : '';
		$value['to'] = Array();

		$to = Array();
		if (isset($_POST[$prefix.'_to'])) {
			$to = explode(';', trim($_POST[$prefix.'_to']));
			// make sure the emails are valid
			foreach ($to as $email) {
				$email = trim($email);
				if (empty($email)) continue;
				$value['to'][] = $email;
			}
		}

		$value['from']    = (isset($_POST[$prefix.'_from'])) ? $_POST[$prefix.'_from'] : '';
		$value['subject'] = (isset($_POST[$prefix.'_subject'])) ? $_POST[$prefix.'_subject'] : '';
		
		$this->processed = $this->setValue($value);

	}//end process()


	/**
	* Ensure the attribute being set is an array
	*
	* @param mixed	&$value	value to be validated
	*
	* @return boolean
	* @access public
	*/
	function validateValue(&$value)
	{
		// cant serilise scalar variables
		if (is_scalar($value)) {
			// see if it is already serilised
			$unser = @unserialize(trim((string) $value));
			if (is_scalar($unser)) return false;
			else $value = $unser;
		}
		
		if (!is_array($value)) return false;
		if (!isset($value['to']))          $test_value['to'] = Array();
		if (!isset($value['from']))        $test_value['from'] = '';
		if (!isset($value['subject']))     $test_value['subject'] = '';
		if (!isset($value['html_format'])) $test_value['html_format'] = '';
		if (!isset($value['text_format'])) $test_value['text_format'] = '';

		$value = serialize($value);
		return true;

	}//end validateValue()
	
	
	/**
	* Use the values stored in this attribute to send an email
	*
	* @param array	$replacements	an array of keyword replacements
	*
	* @return void
	* @access public
	*/
	function sendMail($replacements=Array())
	{
		require_once SQ_FUDGE_PATH.'/general/text.inc';
		require_once SQ_FUDGE_PATH.'/general/www.inc';

		$current_value = @unserialize($this->value);

		// replace keywords in all our values
		$from      = replace_keywords($current_value['from'],        $replacements);
		$subject   = replace_keywords($current_value['subject'],     $replacements);
		$text_body = replace_keywords($current_value['text_format'], $replacements);
		$html_body = replace_keywords($current_value['html_format'], $replacements);

		// replace internal links to pages and images with their full URLs
		$matches = Array();
		preg_match_all('|\./\?a=([0-9]+)|', $html_body, $matches);
		$urls = $GLOBALS['SQ_SYSTEM']->am->getAssetURL(array_unique($matches[1]));
		foreach ($urls as $assetid => $url) {
			$html_body = preg_replace('|\./\?a='.$assetid.'([^0-9])|', $url.'\\1', $html_body);
		}

		require_once 'Mail.php';
		require_once 'Mail/mime.php';

		// new line is \n
		$crlf = "\n";
		$mime = new Mail_mime($crlf);

		$from_addr = preg_replace('|.*\<(.*)\>|', '\\1', $from);

		$mail =& Mail::factory('mail', "-f$from_addr");

		$hdrs = Array('From' => $from, 'Subject' => $subject);

		foreach ($current_value['to'] as $to_email) {

			$to_email = trim($to_email);

			$to_addr = preg_replace('|.*\<(.*)\>|', '\\1', $to_email);

			if (empty($to_email) || !valid_email($to_addr)) continue;

			$mime->setTXTBody(strip_tags($text_body));
			$mime->setHTMLBody($html_body);

			// get() must be called before headers()
			$body = $mime->get();
			$hdrs = $mime->headers($hdrs);

			$msg = $mail->send($to_email, $hdrs, $body);

		}//end foreach

	}//end sendMail()


}//end class

?>