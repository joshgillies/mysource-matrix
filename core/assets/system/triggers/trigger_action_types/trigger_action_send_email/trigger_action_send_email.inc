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
* $Id: trigger_action_send_email.inc,v 1.1 2006/02/10 02:01:35 sdanis Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/general_occasional.inc';
require_once SQ_CORE_PACKAGE_PATH.'/system/triggers/trigger_action/trigger_action.inc';
require_once SQ_SYSTEM_ROOT.'/core/attributes/email_format/email_format.inc';

/**
* Trigger_Action_Add_URL
*
* A trigger action that sets a the value of an asset attribute
*
*
* @author  Sertan Danis <sdanis@squiz.net>
* @version $Revision: 1.1 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Trigger_Action_Send_Email extends Trigger_Action
{


	/**
	* Execute this action
	*
	* @param array	$settings	the stored settings for this action
	* @param array	&$state		the state that describes the asset the action is executing on
	*
	* @return mixed array|boolean
	* @access public
	*/
	function execute($settings, &$state)
	{
		if (empty($state['asset'])) {
			// grab the asset if assetid is given, but not the asset.
			if (empty($state['assetid'])) {
				return FALSE;
			} else {
				$state['asset'] = &$GLOBALS['SQ_SYSTEM']->am->getAsset($state['assetid']);
			}
		}

		if (is_null($state['asset'])) {
			return FALSE;
		}

		$assetid = $state['asset']->id;

		// we only want to process user assets
		if (!$GLOBALS['SQ_SYSTEM']->am->isTypeDecendant($state['asset']->type(), 'user')) {
			return TRUE;
		}

		$email_format_value = $settings['email_format_value'];
		if ($settings['to_owner']) {
			// add owners id
			$unserialised = unserialize($email_format_value);
			$unserialised['to_assetids'][] = $asset_id;
			$email_format_value = serialize($unserialised);
		}

		$keywords = Array('username','first_name', 'last_name', 'email');
		$keyword_replacements = Array();
		foreach ($keywords as $keyword) {
			$keyword_replacements[$keyword] = $state['asset']->attr($keyword);
		}

		$GLOBALS['SQ_SYSTEM']->am->forgetAsset($state['asset']);

		// create a new email format object and send emails
		$email_format_atr =& new Asset_Attribute_Email_Format(0, $email_format_value);
		if (is_null($email_format_atr)) return FALSE;
		$email_format_atr->sendMail($keyword_replacements);

		return Array(
				'assetid'	=> $assetid,
			   );

	}//end execute()


	/**
	* Get the HTML editing interface for this action. Returns the string instead of printing it
	*
	* @param array		$settings		settings that this condition saves in processInterface()
	* @param string		$prefix			unique prefix
	* @param boolean	$write_access	determines whether the interface is editable
	*
	* @return boolean
	* @access public
	*/
	function getInterface($settings, $prefix, $write_access=FALSE)
	{
		ob_start();
		$to_owner = array_get_index($settings, 'to_owner', 0);

		$new_prefix = ereg_replace('\[','_',ereg_replace('\]','_',$prefix));
		$email_format_value = array_get_index($settings, 'email_format_value', Array());

		if ($write_access) {
			?>
			<?php
				echo '<strong>'.translate('trigger_send_to_active_user').'</strong>';
				check_box($prefix.'[to_owner]', 1, $to_owner);
				echo '<br />'.translate('trigger_send_to_active_user_note').'<br /><br >';
				$atr_email_format =& new Asset_Attribute_Email_Format(0, $email_format_value);
				$atr_email_format->paint($new_prefix, FALSE, TRUE);

				hidden_field($prefix.'[new_prefix]',$new_prefix);
			?>
			<?php
		} else {
			?>
			<strong><?php echo translate('trigger_send_to_active_user'); ?></strong>
			<img src="<?php echo sq_web_path('lib'); ?>/web/images/<?php echo ($to_owner ? 'tick' : 'cross'); ?>.gif" width="15" height="15" />
			<?php
			// print email contents
			$atr_email_format =& new Asset_Attribute_Email_Format(0, $email_format_value);
			$atr_email_format->paint($new_prefix, TRUE, TRUE);
		}

		return ob_get_clean();

	}//end getInterface()


	/**
	* Function that handles the conversion of interface to settings
	* together with settings it is expected to populate the hash object
	*
	* @param array	&$settings		a container for any data the action might want to save
	* @param string	$request_data	array of data corresponding to this action as
	*								specified by the $prefix in getInterface.
	*
	* @return boolean
	* @access public
	*/
	function processInterface(&$settings, $request_data)
	{

		// save other settings
		$settings['to_owner'] = array_get_index($request_data, 'to_owner', FALSE);

		// create a wysiwyg compatable prefix
		$new_prefix = ereg_replace('\[','_',ereg_replace('\]','_',array_get_index($request_data, 'new_prefix', '')));

		$email_format =& new Asset_Attribute_Email_Format();
		$email_format->process($new_prefix);
		$settings['email_format_value'] = $email_format->value;

		return FALSE;

	}//end processInterface()


	/**
	* Get the list of locks that the action needs acquire before executing
	*
	* @param array	$settings	the stored settings for this action
	* @param array	&$state		the state that describes the asset the action is executing on
	*
	* @return array
	* @access public
	*/
	function getLocks($settings, &$state)
	{
		return Array($state['assetid'] => Array('lookups'));

	}//end getLocks()


}//end class

?>
