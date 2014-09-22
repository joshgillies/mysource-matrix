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
* $Id: design_area_login_form.inc,v 1.28 2012/08/30 01:09:06 ewang Exp $
*
*/


require_once SQ_CORE_PACKAGE_PATH.'/designs/design_area/design_area.inc';

/**
* Design_Area_Login_Form
*
* Purpose
*
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Revision: 1.28 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Design_Area_Login_Form extends Design_Area
{


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
		// make sure that this var is being painted only within its valid sections
		switch ($var) {
			case 'form_open' :
			case 'form_close' :
			case 'submit_button' :
				if ($this->_tmp['painting_section'] != 'login' && $this->_tmp['painting_section'] != 'logout') {
					trigger_localised_error('CORE0178', sprintf(translate('The "%1$s" var can only be printed inside the login or logout sections of the login_form design area "%2$s"'), $var, $this->attr('id_name')), E_USER_WARNING)
					return FALSE;
				}
			break;

			case 'login_message' :
			case 'username_box' :
			case 'password_box' :
				if ($this->_tmp['painting_section'] != 'login') {
					trigger_localised_error('CORE0179', sprintf(translate('The "%1$s" var can only be printed inside the login section of the login_form design area "%2$s"'), $var, $this->attr('id_name')), E_USER_WARNING)
					return FALSE;
				}
			break;

		}// end switch

		// this is needed by a couple of vars
		$referer = '(isset($_POST[\'SQ_LOGIN_REFERER\']) ? $_POST[\'SQ_LOGIN_REFERER\'] : (empty($_SERVER[\'HTTP_REFERER\']) ? \'\' : $_SERVER[\'HTTP_REFERER\']))';

		switch ($var) {
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
					$qry_str_extra = '&amp;<'.'?php echo htmlentities(preg_replace(\'/SQ_ACTION=[^\&]+\&?/\', \'\', $_SERVER[\'QUERY_STRING\'])); ?'.'>';
				}

				// open login form
				if ($this->_tmp['painting_section'] == 'login') {
					echo '
						<form action="<', '?php echo $_SERVER[\'PHP_SELF\']; ?','>?SQ_ACTION=login'.$qry_str_extra.'" id="'.$this->attr('id_name').'_login_prompt" method="post" onsubmit="return checkEntry();">
					';
				// open logout form
				} else {
					echo '<form action="<', '?php echo $_SERVER[\'PHP_SELF\']; ?','>?SQ_ACTION=logout'.$qry_str_extra.'" id="'.$this->attr('id_name').'_login_prompt" method="post">
					<input type="hidden" name="SQ_LOGIN_REFERER"  value="<', '?php echo htmlspecialchars('.$referer.'); ?', '>" />';
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
					unset($'.$this->attr('id_name').'_post_vars[\'log_in_out_button\']);
					var_to_hidden_field($'.$this->attr('id_name').'_post_vars);
					?', '>';
				}// end if

			break;

			case 'form_close' :
				echo '
					</form>
					<script type="text/javascript"><!--
					//<![CDATA[
						var loginForm = document.forms[\''.$this->attr('id_name').'_login_prompt\'];
						function checkEntry() {
							return (loginForm.SQ_LOGIN_USERNAME.value.length > 0 && loginForm.SQ_LOGIN_PASSWORD.value.length > 0);
						}
					//]]> -->
					</script>
				';
			break;

			case 'system_messages' :
				echo '<', '?php echo implode(\'<br />\', $GLOBALS[\'SQ_SYSTEM\']->messages()); ?', '>';
			break;

			case 'username_box' :
				require_once SQ_LIB_PATH.'/html_form/html_form.inc';
				$extras = 'onfocus="this.select();"';
				foreach ($args As $k => $v) {
					if ($k == 'size') continue;
					$extras .= ' '.$k.'="'.htmlspecialchars($v).'"';
				}
				echo '<input type="hidden" name="SQ_LOGIN_KEY"      value="<', '?php echo $GLOBALS[\'SQ_SYSTEM\']->generateLoginKey(); ?', '>" />
					  <input type="hidden" name="SQ_LOGIN_REFERER"  value="<', '?php echo htmlspecialchars('.$referer.'); ?', '>" />';

				text_box('SQ_LOGIN_USERNAME', '', empty($args['size']) ? 10 : $args['size'], 0, FALSE, $extras);
				if ($this->attr('give_username_focus')) {
					echo '
						<script type="text/javascript"><!--
						//<![CDATA[
							var '.$this->attr('id_name').'_username_box_other_onload = (window.onload) ? window.onload : new Function;
							function set_'.$this->attr('id_name').'_username_box_focus() {
								'.$this->attr('id_name').'_username_box_other_onload();
								document.getElementById("'.$this->attr('id_name').'_login_prompt").elements["SQ_LOGIN_USERNAME"].focus();
							}
							window.onload = set_'.$this->attr('id_name').'_username_box_focus;
						//]]>-->
						</script>
					';
				}
			break;

			case 'password_box' :
				require_once SQ_LIB_PATH.'/html_form/html_form.inc';
				$extras = 'onfocus="this.select();"';
				foreach ($args As $k => $v) {
					if ($k == 'size') continue;
					$extras .= ' '.$k.'="'.htmlspecialchars($v).'"';
				}
				password_box('SQ_LOGIN_PASSWORD', '', empty($args['size']) ? 10 : $args['size'], 0, $extras);
			break;

			case 'submit_button' :
				require_once SQ_LIB_PATH.'/html_form/html_form.inc';
				$extras = '';
				foreach ($args As $k => $v) {
					if ($k == 'value') continue;
					$extras .= ' '.$k.'="'.htmlspecialchars($v).'"';
				}
				submit_button('log_in_out_button', empty($args['value']) ? ucfirst($this->_tmp['painting_section']) : $args['value'], '', $extras);
			break;

			default :
				parent::printVar($var, $args);
		}//end switch

	}//end printVar()


	/**
	* Returns true if this content generated by this design area can be cached
	*
	* @return boolean
	* @access public
	*/
	public function canBeCached()
	{
		return FALSE;

	}//end canBeCached()


	/**
	* Print code that initialises this design area and should always be printed in place of the AREA tag
	*
	* @return void
	* @access protected
	*/
	protected function _printInitCode()
	{
		echo '<', '?php
		if (!empty($GLOBALS[\'SQ_LOGIN_FORM\'])) {
			$'.$this->attr('id_name').'_paint_login = ($GLOBALS[\'SQ_LOGIN_FORM\'][\'paint\'] == \'login\');
		} else {
			$'.$this->attr('id_name').'_paint_login = $GLOBALS[\'SQ_SYSTEM\']->userPublic();
		}
		?', '>';

	}//end _printInitCode()


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
			case 'login_section':
				$this->_tmp['painting_section'] = 'login';
				echo '<', '?php
				if ($'.$this->attr('id_name').'_paint_login) {
					// login section
					?', '>
					';
				foreach ($this->attr('login_contents') as $item) {
					parent::_printContentItemCode($item);
				}
				echo '
					<', '?php
				}//end login section
				?', '>';
				$this->_tmp['painting_section'] = '';
			break;

			case 'logout_section':
				$this->_tmp['painting_section'] = 'logout';
				echo '<', '?php
				if (!$'.$this->attr('id_name').'_paint_login) {
					// logout section
					?', '>
					';
				foreach ($this->attr('logout_contents') as $item) {
					parent::_printContentItemCode($item);
				}
				echo '
					<', '?php
				}//end logout section
				?', '>';
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
		$res[] = 'login_contents';
		$res[] = 'logout_contents';
		return $res;

	}//end getProtectedAttrs()


}//end class
?>
