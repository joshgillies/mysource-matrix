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
* $Id: form_action_set_session_var.inc,v 1.2 2012/08/30 00:58:07 ewang Exp $
*
*/


require_once SQ_PACKAGES_PATH.'/cms/form/form_action/form_action.inc';

require_once SQ_FUDGE_PATH.'/general/general.inc';

/**
* Form_Action_Set_Session_Var
*
* Purpose
*
*
* @author  Wojciech Szymanski <wojtek@squiz.pl>
* @version $Revision: 1.2 $
* @package MySource_Matrix_Packages
* @subpackage cms
*/


class Form_Action_Set_Session_Var extends Form_Action
{


	/**
	* Constructor
	*
	*/
	function __construct()
	{
		parent::__construct();

	}//end constructor


	/**
	* Paint inline interface
	*
	*
	*/
	public static function paintInlineInterface(Form $form, $settings, Backend_Outputter $o, $prefix)
	{
		if (!isset($settings['set_session_var'])) {
			$var_settings = Array();
		} else {
			$var_settings = $settings['set_session_var'];
		}
		$name = array_get_index($var_settings, 'var_name', '');
		$value = array_get_index($var_settings, 'var_value', '');

		// Paint something here!
		?>
		<table class="sq-backend-table" style="width:100%; display:block;">
			<tr>
				<th><div style="width:150px"><?php echo translate('Variable Name');?></div></th>

				<td>
				<?php
				text_box($prefix.'_var_name', $name, 90);
				?>
				</td>
			</tr>
			<tr>
				<th><?php echo translate('Variable Value');?></th>

				<td width="100%">
				<?php
				text_box($prefix.'_var_value', $value, 90);
				?>
				</td>
			</tr>
		</table>
		<?php

	}//end paintInlineInterface()


	/**
	* Process inline interface
	*
	*
	*/
	public static function processInlineInterface(Form $form, &$settings, Backend_Outputter $o, $prefix)
	{
		if (!isset($settings['set_session_var'])) {
			$var_settings = Array();
		} else {
			$var_settings = $settings['set_session_var'];
		}

		$var_settings['var_name'] = array_get_index($_POST, $prefix.'_var_name', '');
		$var_settings['var_value'] = array_get_index($_POST, $prefix.'_var_value', '');

		$settings['set_session_var'] = $var_settings;

		return TRUE;

	}//end processInlineInterface()


	/**
	* Paint summary description
	*
	*
	*/
	public static function paintSummary(Form $form, $settings, Backend_Outputter $o, $prefix)
	{
		if (!isset($settings['set_session_var'])) {
			$var_settings = Array();
		} else {
			$var_settings = $settings['set_session_var'];
		}
		$name = array_get_index($var_settings, 'var_name', '');
		$value = array_get_index($var_settings, 'var_value', '');

		?><table class="no-borders">
			<colgroup>
				<col width="80" />
				<col/>
			</colgroup>
			<tbody>
				<tr>
					<td class="sq-backend-table-cell" style="vertical-align: top"><p><strong><?php echo translate('Variable Name');?></strong></p></td>

					<td class="sq-backend-table-cell" style="vertical-align: top"><p><?php echo $name; ?></p></td>
				</tr>
				<tr>
					<td class="sq-backend-table-cell" style="vertical-align: top"><p><strong><?php echo translate('Set Value');?></strong></p></td>

					<td class="sq-backend-table-cell" style="vertical-align: top"><p><?php echo $value; ?></p></td>
				</tr>
			</tbody>
		</table>
		<?php

	}//end paintSummary()


	/**
	* Execute form action
	*
	*/
	public static function execute(Form $form, $settings)
	{
		$var_settings = $settings['set_session_var'];
		$found = 0;

		// We will replace all responses, including raw values.
		// We will also replace keywords relating to the form submission asset
		// (eg. submission assetid, time, IP address)
		$matches = Array();
		$found = preg_match_all('/%(response_(\\d+_)?q\\d+(_raw)?)%/U', $var_settings['var_value'], $set_matches, PREG_SET_ORDER);
		$matches += $set_matches;
		$found = preg_match_all('/%(form_submission_.*)%/U', $var_settings['var_value'], $set_matches, PREG_SET_ORDER);
		$matches = array_merge($matches, $set_matches);

		foreach ($matches as $match) {
			if (empty($match)) continue;
			$var_settings['var_value'] = preg_replace('/%'.$match[1].'%/U', html_entity_decode($form->_getThankYouKeywordReplacement($match[1])), $var_settings['var_value']);
		}

		$_SESSION[SQ_SESSION_SANDBOX_INDEX][$var_settings['var_name']] = $var_settings['var_value'];

		return TRUE;

	}//end execute()


	/**
	* Execute form action
	*
	*/
	public static function isValid(Form $form, $settings)
	{
		if (!isset($settings['set_session_var'])) {
			$var_settings = Array();
		} else {
			$var_settings = $settings['set_session_var'];
		}
		self::_fillDefaultValues($var_settings);

		// Check if valid and return FALSE
		if (!isset($var_settings['var_name']) || empty($var_settings['var_name'])) {
			return FALSE;
		}//end if

		if (!isset($var_settings['var_value'])) return FALSE;

		return TRUE;

	}//end isValid()


	/**
	* Fill any missing values from the session settings with defaults
	*
	* @param array	&$session_format	The session settings
	*
	* @return void
	*/
	private static function _fillDefaultValues(&$session_format)
	{
		if (!isset($session_format['var_value'])) {
			$session_format['var_value'] = '';
		}
		if (!isset($session_format['var_name'])) {
			$session_format['var_name'] = '';
		}

	}//end _fillDefaultValues()


}//end class

?>
