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
* $Id: design_area_login_form.inc,v 1.10 2004/06/29 07:38:10 tbarrett Exp $
* $Name: not supported by cvs2svn $
*/


require_once SQ_CORE_PACKAGE_PATH.'/designs/design_area/design_area.inc';

/**
* Design_Area_Login_Form
*
* Purpose
*
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Design_Area_Login_Form extends Design_Area
{

	/*
	* Outputs the var either as straight HTML or PHP code, depending on the type
	*
	* @param string	$var			the name of the var to print out
	* @param Array	$args			array of any extra arguments that existed in the print command
	* @param string	$value_prefix	PHP code to wrap put before the value before outputting it
	* @param string	$value_suffix	PHP code to wrap put after  the value before outputting it
	*
	* @access public
	*/
	function printVar($var, $args, $value_prefix='', $value_suffix='')
	{

		// make sure that this var is being painted only within it's valid sections
		switch($var) {
			case 'form_open' :
			case 'form_close' :
			case 'submit_button' :
				if ($this->_tmp['painting_section'] != 'login' && $this->_tmp['painting_section'] != 'logout') {
					trigger_error('The "'.$var.'" var can only be printed inside the login or logout sections of the login_form design area "'.$asset->attr('id_name').'"', E_USER_WARNING);
					return false;
				}
				break;

			case 'login_message' :
			case 'username_box' :
			case 'password_box' :
				if ($this->_tmp['painting_section'] != 'login') {
					trigger_error('The "'.$var.'" var can only be printed inside the login section of the login_form design area "'.$asset->attr('id_name').'"', E_USER_WARNING);
					return false;
				}
				break;

		}// end switch

		// this is needed by a couple of vars
		$referer = '(isset($_POST[\'SQ_LOGIN_REFERER\']) ? $_POST[\'SQ_LOGIN_REFERER\'] : (empty($_SERVER[\'HTTP_REFERER\']) ? \'\' : $_SERVER[\'HTTP_REFERER\']))';

		switch($var) {
			case 'login_heading' :
				echo '<', '?php if (!empty($GLOBALS[\'SQ_LOGIN_FORM\'])) { echo ', $this->_escapeVar('$GLOBALS[\'SQ_LOGIN_FORM\'][\'heading\']', $args), '; } ?', '>';
				break;

			case 'login_message' :
				echo '<', '?php if (!empty($GLOBALS[\'SQ_LOGIN_FORM\'])) { echo ', $this->_escapeVar('$GLOBALS[\'SQ_LOGIN_FORM\'][\'msg\']', $args), '; } ?', '>';
				break;

			case 'referer' :
				echo '<', '?php echo ', $this->_escapeVar($referer, $args), '; ?', '>';
				break;

			case 'form_open' :

				$qry_str_extra = '';
				if ($this->attr('forward_www_vars')) {
					$qry_str_extra = '&<'.'?php echo preg_replace(\'/SQ_ACTION=[^\&]+\&?/\', \'\', $_SERVER[\'QUERY_STRING\']); ?'.'>';
				}

				// open login form
				if ($this->_tmp['painting_section'] == 'login') {
					echo '<form action="<', '?php echo $_SERVER[\'PHP_SELF\']; ?','>?SQ_ACTION=login'.$qry_str_extra.'" name="'.$this->attr('id_name').'_login_prompt" method="POST" onSubmit="javascript: return (this.elements[\'SQ_LOGIN_USERNAME\'].value.length > 0 && this.elements[\'SQ_LOGIN_PASSWORD\'].value.length > 1);">
					<input type="hidden" name="SQ_LOGIN_KEY"      value="<', '?php echo $GLOBALS[\'SQ_SYSTEM\']->generateLoginKey(); ?', '>">
					<input type="hidden" name="SQ_LOGIN_REFERER"  value="<', '?php echo htmlspecialchars('.$referer.'); ?', '>">';
				// open logout form
				} else {
					echo '<form action="<', '?php echo $_SERVER[\'PHP_SELF\']; ?','>?SQ_ACTION=logout'.$qry_str_extra.'" name="'.$this->attr('id_name').'_login_prompt" method="POST">
					<input type="hidden" name="SQ_LOGIN_REFERER"  value="<', '?php echo htmlspecialchars('.$referer.'); ?', '>">';
				}

				if ($this->attr('forward_www_vars')) {
					echo '<', '?php
					// Take all the post vars and whack em in the form - important ones are removed
					require_once SQ_LIB_PATH.\'/html_form/html_form.inc\';
					$'.$this->attr('id_name').'_post_vars = $_POST;
					unset($'.$this->attr('id_name').'_post_vars[\'SQ_LOGIN_USERNAME\']);
					unset($'.$this->attr('id_name').'_post_vars[\'SQ_LOGIN_PASSWORD\']);
					unset($'.$this->attr('id_name').'_post_vars[\'SQ_LOGIN_KEY\']);
					unset($'.$this->attr('id_name').'_post_vars[\'SQ_LOGIN_REFERER\']);
					unset($'.$this->attr('id_name').'_post_vars[\'SQ_ACTION\']);
					var_to_hidden_field($'.$this->attr('id_name').'_post_vars);
					?', '>';
				}// end if

				break;

			case 'form_close' :
				echo '</form>';
				break;

			case 'system_messages' :
				echo '<', '?php echo implode(\'<br/>\', $GLOBALS[\'SQ_SYSTEM\']->messages()); ?', '>';
				break;

			case 'username_box' :
				require_once SQ_LIB_PATH.'/html_form/html_form.inc';
				$extras = 'onFocus="javascript: this.select();"';
				foreach($args As $k => $v) {
					if ($k = 'size') continue;
					$extras .= ' '.$k.'="'.htmlspecialchars($v).'"';
				}
				text_box('SQ_LOGIN_USERNAME', '', empty($args['size']) ? 10 : $args['size'], 0, false, $extras);
				if ($this->attr('give_username_focus')) {
					echo '<script language="javascript" type="text/javascript">
						function set_'.$this->attr('id_name').'_username_box_focus() {
							'.$this->attr('id_name').'_username_box_other_onload();
							document.'.$this->attr('id_name').'_login_prompt.elements[\'SQ_LOGIN_USERNAME\'].focus();
						}
						var '.$this->attr('id_name').'_username_box_other_onload = (window.onload) ? window.onload : new Function;
						window.onload = set_'.$this->attr('id_name').'_username_box_focus;
						</script>';
				}


				break;

			case 'password_box' :
				require_once SQ_LIB_PATH.'/html_form/html_form.inc';
				$extras = 'onFocus="javascript: this.select();"';
				foreach($args As $k => $v) {
					if ($k = 'size') continue;
					$extras .= ' '.$k.'="'.htmlspecialchars($v).'"';
				}
				password_box('SQ_LOGIN_PASSWORD', '', empty($args['size']) ? 10 : $args['size'], 0, $extras);
				break;

			case 'submit_button' :
				require_once SQ_LIB_PATH.'/html_form/html_form.inc';
				$extras = '';
				foreach($args As $k => $v) {
					if ($k == 'value') continue;
					$extras .= ' '.$k.'="'.htmlspecialchars($v).'"';
				}
				submit_button('log_in_out_button', empty($args['value']) ? ucfirst($this->_tmp['painting_section']) : $args['value'], '', $extras);
				break;

			default :
				parent::printVar($var, $args);
		}// end switch

	}//end printVar()


}//end class
?>
