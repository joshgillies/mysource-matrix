<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ACN 084 670 600                                                    |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.net) so we may provide   |
* | you a copy.                                                        |
* +--------------------------------------------------------------------+
*
* $Id: trigger_action_set_design.inc,v 1.18 2007/12/13 04:11:17 hnguyen Exp $
*
*/

require_once SQ_LIB_PATH.'/html_form/html_form.inc';


/**
* Trigger Action Set Design
*
* A trigger action that sets the default design of an asset
*
*
* @author   Andrei Railean <arailean@squiz.net>
* @author   Robert Howard <rhoward@squiz.net>
* @version $Revision: 1.18 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Trigger_Action_Set_Design extends Trigger_Action
{


	/**
	* Execute this action
	* Returns an array of data about what it did, or false on error
	*
	* The settings used by this action are in the form:
	* <PRE>
	* Array(
	*       'assetid'		=> int // the assetid of the design that will be applied to the asset stored in $state
	*		'design_type'	=> string // the design type (either design::system or design::user)
	* 		'design_name'	=> string // the name for the user defined design
	*       );
	* </PRE>
	*
	* @param array	$settings	the stored settings for this action
	* @param array	&$state		the state that describes the asset the action is executing on
	*
	* @return mixed array|boolean
	* @access public
	*/
	public static function execute($settings, &$state)
	{
		// check settings, state
		if (empty($settings['assetid'])) {
			// if no settings, fail
			return FALSE;
		}

		if (!$GLOBALS['SQ_SYSTEM']->am->assetExists($settings['assetid'])) {
			return FALSE;
		}

		$design_type = array_get_index($settings, 'design_type', 'design::system');
		$design_name = array_get_index($settings, 'design_name', '::user_define');

		// state
		if (empty($state['asset'])) {
			// grab the asset if assetid is given, but not the asset.
			if (empty($state['assetid'])) {
				return FALSE;
			} else {
				$state['asset'] = $GLOBALS['SQ_SYSTEM']->am->getAsset($state['assetid']);
			}
		}


		$am = $GLOBALS['SQ_SYSTEM']->am;

		// setting the default frontend design
		if ($design_type == 'design::system') {
			$design_final_name = 'design::system::frontend';
		} else if ($design_type == 'design::user') {
			$design_final_name = 'design::user::'.$design_name;
		} else {
			$design_final_name = $design_type;
		}

		// get all system_design::frontend links attached to the asset
		$design_links = $am->getLinks($state['assetid'], SQ_LINK_NOTICE, 'design', FALSE, 'major', $design_final_name);
		$link_already_created = FALSE;

		if (!empty($design_links)) {

			// Wipe them out. All of them.
			foreach ($design_links as $link) {

				// if the new design is already linked, leave it alone. delete duplicates if found
				if ($link['minorid'] == $settings['assetid'] && !$link_already_created) {
					// so that we don't try and recreate the link later if it already exists
					$link_already_created = TRUE;
				} else {
					if (!$am->deleteAssetLink($link['linkid'])) {
						// if we try to create another system_design::frontend while one already exists, we
						// end up in a whole world of pain. Bail now.
						return FALSE;
					}
				}

			}
		}//end if(!empty())

		// create a link between the asset (major) and the design (minor)
		if (!$link_already_created) {
			$design = $am->getAsset($settings['assetid']);
			$link_create_status = $am->createAssetLink($state['asset'], $design, SQ_LINK_NOTICE, $design_final_name);

			if (!$link_create_status) {
				// couldn't create link, die
				return FALSE;
			}
		}

		// Do it by HIPO so that the children assets are also affected.
		$hh = $GLOBALS['SQ_SYSTEM']->getHipoHerder();
		$vars = Array('assetids' => Array($state['asset']->id));
		$hh->freestyleHipo('hipo_job_update_lookups', $vars);
		return Array(
				'assetid'	=> $state['asset']->id,
				'designid'	=> $settings['assetid'],
				'code'		=> 'system::frontend',
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
	public static function getInterface($settings, $prefix, $write_access=FALSE)
	{

		if (empty($settings['assetid'])) {
			// set to default
			$selected_assetid = '0';
		} else {
			$selected_assetid = $settings['assetid'];
		}

		$design_type = array_get_index($settings, 'design_type', '');
		$design_name = array_get_index($settings, 'design_name', '');

		// check if the selected asset is valid
		if ($selected_assetid && !$GLOBALS['SQ_SYSTEM']->am->assetExists($selected_assetid)) {
			$selected_assetid = 0;
		}

		if (!$write_access) {
			$form_element_extras = 'disabled="disabled"';
		} else {
			$form_element_extras = '';
		}

		$design_types	= Array (
							'design::system::frontend'	=> 'Frontend Design',
							'design::system::login'		=> 'Login Design',
							'design::user'				=> 'User Defined Design',
						  );

		// begin buffering basic options

		$disabled = ($design_type == 'design::user') ? '' : 'disabled="disabled"';
		$extra = 'onChange="if(this.value == \'design::user\') { document.getElementById(\''.$prefix.'[design_name]'.'\').disabled=\'\';}
				else { document.getElementById(\''.$prefix.'[design_name]'.'\').disabled=\'disabled\'; }"';
		ob_start();
			if ($write_access) {
				asset_finder($prefix.'[assetid]', $selected_assetid, Array('design' => 'D'));
				echo '<br /><br />';
				echo translate('trigger_set_design_select_design_type');
				combo_box($prefix.'[design_type]', $design_types, FALSE, $design_type, 0, $extra);
				echo '<br /><br />';
				echo translate('trigger_set_design_set_user_design_name');
				text_box($prefix.'[design_name]', $design_name, '', '', FALSE, $disabled);
			} else {
				if ($selected_assetid) {
					$selected_assetname = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo(Array($selected_assetid), Array(), TRUE, 'name');
					text_box($prefix.'[assetid]', $selected_assetname[$selected_assetid].' (#'.$selected_assetid.')', '', '', FALSE, $form_element_extras);
				} else {
					text_box($prefix.'[assetid]', '', '', '', FALSE, $form_element_extras);
				}
			}

			$basic_part_1 = ob_get_contents();
		ob_end_clean();

		return translate('trigger_set_design_basic', $basic_part_1);

	}//end getInterface()


	/**
	* Function that handles the conversion of interface to settings
	* together with settings it is expected to populate the hash object
	*
	* @param array	&$settings		a container for any data the action might want to save
	* @param string	$request_data	array of data corresponding to this action
	*								as specified by the $prefix in getInterface.
	*
	* @return boolean
	* @access public
	*/
	public static function processInterface(&$settings, $request_data)
	{
		// make sure the assetid isn't blank
		if (empty($request_data['assetid']['assetid'])) {
			return 'Design Asset id is not specified';
		}

		// check if the status is valid
		$settings['assetid'] = $request_data['assetid']['assetid'];
		$settings['design_type'] = $request_data['design_type'];
		if (!empty($request_data['design_name'])) {
			$settings['design_name'] = $request_data['design_name'];
		}

		return FALSE;

	}//end processInterface()


	/**
	* Get the list of locks that the action needs to acquire before executing
	*
	* @param array	$settings	a container for any data the action might want to save
	* @param array	&$state		the state that describes the asset the action is executing on
	*
	* @return array
	* @access public
	*/
	public static function getLocks($settings, &$state)
	{
		return Array(
				$state['assetid']	=> Array(
										'links',
										'lookups',
									   ),
			   );

	}//end getLocks()


}//end class

?>
