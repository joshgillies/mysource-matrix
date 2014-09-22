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
* $Id: design_area_password_change_form.inc,v 1.10 2012/08/30 01:09:06 ewang Exp $
*
*/


require_once SQ_CORE_PACKAGE_PATH.'/designs/design_area/design_area.inc';

/**
* Design_Area_Password_Change_Form
*
* Purpose
*	Show the change-password interface in the change-password design
*
* @author  Nathan de Vries <ndvries@squiz.net>
* @author  Robert Howard <rhoward@squiz.net>
* @version $Revision: 1.10 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Design_Area_Password_Change_Form extends Design_Area
{


	/**
	* Constructor
	*
	* @param string	$assetid	The asset ID to load
	*
	*/
	function __construct($assetid=0)
	{
		parent::__construct($assetid);

	}//end constructor


	/**
	* Outputs the var either as straight HTML or PHP code, depending on the type
	*
	* @param string	$var			the name of the var to print out
	* @param array	$args			array of any extra arguments that existed in the print command
	* @param string	$value_prefix	PHP code to wrap put before the value before outputting it
	* @param string	$value_suffix	PHP code to wrap put after  the value before outputting it
	*
	* @return void
	* @access public
	*/
	public function printVar($var, Array $args, $value_prefix='', $value_suffix='')
	{

		// make sure that this var is being painted only within it's valid sections
		switch ($var) {
			case 'form_open' :
			case 'form_close' :
			case 'submit_button' :
			case 'password_change_message' :
			case 'password_old_box' :
			case 'password_new_box' :
			case 'password_confirm_box' :
				if ($this->_tmp['painting_section'] != 'password_change') {
					trigger_localised_error('CORE0179', sprintf(translate('The "%1$s" var can only be printed inside the login section of the login_form design area "%2$s"'), $var, $this->attr('id_name')), E_USER_WARNING)
					return FALSE;
				}
			break;

		}// end switch

		switch ($var) {
			case 'password_change_heading' :
				echo '<', '?php if (!empty($GLOBALS[\'SQ_PASSWORD_CHANGE_FORM\'])) { echo ', $this->_escapeVar('$GLOBALS[\'SQ_PASSWORD_CHANGE_FORM\'][\'heading\']', $args), '; } ?', '>';
			break;

			case 'password_change_message' :
				echo '<', '?php if (!empty($GLOBALS[\'SQ_PASSWORD_CHANGE_FORM\'])) { echo ', $this->_escapeVar('$GLOBALS[\'SQ_PASSWORD_CHANGE_FORM\'][\'msg\']', $args), '; } ?', '>';
			break;

			case 'form_open' :

				$qry_str_extra = '';
				if ($this->attr('forward_www_vars')) {
					$qry_str_extra = '&amp;<'.'?php echo htmlentities(preg_replace(\'/SQ_ACTION=[^\&]+\&?/\', \'\', $_SERVER[\'QUERY_STRING\'])); ?'.'>';
				}

				// open change password form
				if ($this->_tmp['painting_section'] == 'password_change') {
					echo '<form action="<', '?php echo $_SERVER[\'PHP_SELF\']; ?','>?SQ_ACTION=password_change'.$qry_str_extra.'" id="'.$this->attr('id_name').'_password_change_prompt" method="post" onsubmit="return checkEntry();">';
				// open logout form
				} else {
					echo '<form action="<', '?php echo $_SERVER[\'PHP_SELF\']; ?','>?SQ_ACTION=logout'.$qry_str_extra.'" id="'.$this->attr('id_name').'_password_change_prompt" method="post">';
				}

				if ($this->attr('forward_www_vars')) {
					echo '<', '?php
					// Take all the post vars and whack em in the form - important ones are removed
					require_once SQ_LIB_PATH.\'/html_form/html_form.inc\';
					$'.$this->attr('id_name').'_post_vars = $_POST;
					unset($'.$this->attr('id_name').'_post_vars[\'SQ_PASSWORD_CHANGE_OLD\']);
					unset($'.$this->attr('id_name').'_post_vars[\'SQ_PASSWORD_CHANGE_NEW\']);
					unset($'.$this->attr('id_name').'_post_vars[\'SQ_PASSWORD_CHANGE_CONFIRM\']);
					unset($'.$this->attr('id_name').'_post_vars[\'SQ_ACTION\']);
					var_to_hidden_field($'.$this->attr('id_name').'_post_vars);
					?', '>';
				}// end if

			break;

			case 'form_close' :
				echo '</form>
				<script type="text/javascript"><!--
				//<![CDATA[
				var passwordChangeForm = document.forms[\''.$this->attr('id_name').'_password_change_prompt\'];
					function checkEntry() {
						return (passwordChangeForm.SQ_PASSWORD_CHANGE_OLD.value.length > 0 && passwordChangeForm.SQ_PASSWORD_CHANGE_NEW.value.length > 0 && passwordChangeForm.SQ_PASSWORD_CHANGE_CONFIRM.value.length > 0);
					}
				//]]> -->
				</script>
				';
			break;

			case 'system_messages' :
				echo '<', '?php echo implode(\'<br/>\', $GLOBALS[\'SQ_SYSTEM\']->messages()); ?', '>';
			break;

			case 'password_old_box' :
				require_once SQ_LIB_PATH.'/html_form/html_form.inc';
				$extras = 'onfocus="this.select();"';
				foreach ($args As $k => $v) {
					if ($k = 'size') continue;
					$extras .= ' '.$k.'="'.htmlspecialchars($v).'"';
				}
				password_box('SQ_PASSWORD_CHANGE_OLD', '', empty($args['size']) ? 10 : $args['size'], 0, $extras);
				if ($this->attr('give_password_old_focus')) {
					echo '<script type="text/javascript"><!--
						//<![CDATA[
						function set_'.$this->attr('id_name').'_password_old_box_focus() {
							document.getElementById("'.$this->attr('id_name').'_password_change_prompt").elements["SQ_PASSWORD_CHANGE_OLD"].focus();
						}
						window.onload = set_'.$this->attr('id_name').'_password_old_box_focus;
						//]]>
						-->
						</script>';
				}
			break;

			case 'password_new_box' :
				require_once SQ_LIB_PATH.'/html_form/html_form.inc';
				$extras = 'onfocus="this.select();"';
				foreach ($args As $k => $v) {
					if ($k = 'size') continue;
					$extras .= ' '.$k.'="'.htmlspecialchars($v).'"';
				}
				password_box('SQ_PASSWORD_CHANGE_NEW', '', empty($args['size']) ? 10 : $args['size'], 0, $extras);
			break;

			case 'password_confirm_box' :
				require_once SQ_LIB_PATH.'/html_form/html_form.inc';
				$extras = 'onfocus="this.select();"';
				foreach ($args As $k => $v) {
					if ($k = 'size') continue;
					$extras .= ' '.$k.'="'.htmlspecialchars($v).'"';
				}
				password_box('SQ_PASSWORD_CHANGE_CONFIRM', '', empty($args['size']) ? 10 : $args['size'], 0, $extras);
			break;

			case 'submit_button' :
				require_once SQ_LIB_PATH.'/html_form/html_form.inc';
				$extras = '';
				foreach ($args As $k => $v) {
					if ($k == 'value') continue;
					$extras .= ' '.$k.'="'.htmlspecialchars($v).'"';
				}
				submit_button('change_password_logout_button', empty($args['value']) ? ucfirst($this->_tmp['painting_section']) : $args['value'], '', $extras);
			break;

			case 'password_rules_note':
				echo '<','?php
					echo $GLOBALS["SQ_SYSTEM"]->user->getPasswordRulesDescription();
					?', '>';
			break;

			default :
				parent::printVar($var, $args);
		}//end switch

	}//end printVar()


	/**
	* Print code that outputs a given element from this DA's contents
	*
	* @param array	$element	An entry from our contents array
	*
	* @return void
	* @access private
	*/
	protected function _printContentItemCode(Array $element)
	{
		switch (array_get_index($element, 'operation')) {
			case 'password_change_section':
				$this->_tmp['painting_section'] = 'password_change';
				foreach ($this->attr('password_change_contents') as $item) {
					parent::_printContentItemCode($item);
				}
				$this->_tmp['painting_section'] = '';
			break;

			default :
				return parent::_printContentItemCode($element);

		}//end switch

	}//end _printContentItemCode()


	/**
	* Get the names of attributes for this DA that cannot be set in the parse file
	*
	* Design Area attributes that are not mentioned in the parse file are normally set to defaults
	* when parsing the design.  Attributes returned by this function are exempted from that process.
	*
	* @return array
	* @access public
	*/
	public function getProtectedAttrs()
	{
		$res = parent::getProtectedAttrs();
		$res[] = 'password_change_contents';
		return $res;

	}//end getProtectedAttrs()


}//end class
?>
