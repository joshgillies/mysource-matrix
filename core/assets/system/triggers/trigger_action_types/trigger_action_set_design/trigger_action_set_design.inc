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
* $Id: trigger_action_set_design.inc,v 1.4 2005/03/02 03:48:28 rhoward Exp $
*
*/

require_once SQ_LIB_PATH.'/html_form/html_form.inc';
//

/**
* Trigger Action Set Design
*
* A trigger action that sets the default design of an asset
*
*
* @author   Andrei Railean <arailean@squiz.net>
* @author   Robert Howard <rhoward@squiz.net>
* @version $Revision: 1.4 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Trigger_Action_Set_Design extends Trigger_Action
{

	/**
	* Execute this action
	*
	* The settings used by this action are in the form:
	* <PRE>
	* Array(
	*       'assetid' => int        // the assetid of the design that will be applied to the asset stored in $state
	*       );
	* </PRE>
	*
	* @param array(string=>mixed)   $settings   the stored settings for this action
	* @param array(string=>mixed)   &$state the state that describes the asset the action is executing on
	*
	* @return boolean
	* @access public
	*/
	function execute($settings, &$state)
	{
		// check settings, state
		if (empty($settings['assetid'])) {
			// if no settings, fail
			return false;
		}

		if (!$GLOBALS['SQ_SYSTEM']->am->assetExists($settings['assetid'])) {
			return false;
		}

		// state
		if (empty($state['asset'])) {
			// grab the asset if assetid is given, but not the asset.
			if (empty($state['assetid'])) {
				return false;
			} else {
				$state['asset'] = &$GLOBALS['SQ_SYSTEM']->am->getAsset($state['assetid']);
			}
		}


		$am = $GLOBALS['SQ_SYSTEM']->am;

		// setting the default frontend design
		$design_name = 'design::system::frontend';

		// get all system_design::frontend links attached to the asset
		$design_links = $am->getLinks($state['assetid'], SQ_LINK_NOTICE, 'design', false, 'major', $design_name);
		$link_already_created = false;

		if (!empty($design_links)) {

			// Wipe them out. All of them.
			foreach ($design_links as $link) {

				// if the new design is already linked, leave it alone. delete duplicates if found
				if ($link['minorid'] == $settings['assetid'] && !$link_already_created) {
					// so that we don't try and recreate the link later if it already exists
					$link_already_created = true;
				} else {
					if (!$am->deleteAssetLink($link['linkid'])) {
						// if we try to create another system_design::frontend while one already exists, we
						// end up in a whole world of pain. Bail now.
						return false;
					}
				}

			}
		}//end if(!empty())

		// create a link between the asset (major) and the design (minor)
		if (!$link_already_created) {
			$link_create_status = $am->createAssetLink(
				$state['asset'],
				$am->getAsset($settings['assetid']),
				SQ_LINK_NOTICE,
				$design_name
			);

			if (!$link_create_status) {
				// couldn't create link, die
				return false;
			}
		}

		// take the link and insert the relevant entries into the lookup tables
		$state['asset']->updateLookups();

		return true;

	}//end execute()


	/**
	* Get the HTML editing interface for this action. Returns the string instead of printing it
	*
	* @param Array(string=>mixed)   $settings      settings that this condition saves in processInterface()
	* @param string                 $prefix        unique prefix
	* @param boolean                $write_access  determines whether the interface is editable
	*
	* @access public
	* @return boolean
	*/
	function getInterface($settings, $prefix, $write_access=false)
	{
		if (empty($settings['assetid'])) {
			// set to default
			$selected_assetid = '0';
		} else {
			$selected_assetid = $settings['assetid'];
		}

		// check if the selected asset is valid
		if ($selected_assetid && !$GLOBALS['SQ_SYSTEM']->am->assetExists($selected_assetid)) {
			$selected_assetid = 0;
		}

		if (!$write_access) {
			$form_element_extras = 'disabled="disabled"';
		} else {
			$form_element_extras = '';
		}

		// capture anything printed
		ob_start();

		?>
		Set the asset's design to:
		<?php
		if ($write_access) {
			asset_finder($prefix.'[assetid]', $selected_assetid, Array('design' => 'D'));
		} else {
			if ($selected_assetid) {
				$selected_assetname = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo(Array($selected_assetid), Array(), true, 'name');
				text_box($prefix.'[assetid]', $selected_assetname[$selected_assetid].' (#'.$selected_assetid.')', '', '', false, $form_element_extras);
			} else {
				text_box($prefix.'[assetid]', '', '', '', false, $form_element_extras);
			}
		}


		return ob_get_clean();

	}//end getInterface()


	/**
	* Function that handles the conversion of interface to settings
	* together with settings it is expected to populate the hash object
	*
	* @param Array          $settings       a container for any data the action might want to save
	* @param string         $request_data   array of data corresponding to this action as specified by the $prefix in getInterface.
	*
	* @access public
	* @return boolean
	*/
	function processInterface(&$settings, $request_data)
	{
		// make sure the assetid isn't blank
		if (empty($request_data['assetid']['assetid'])) {
			return false;
		}

		// check if the status is valid
		$settings['assetid'] = $request_data['assetid']['assetid'];
		return true;

	}//end processInterface()


}//end class

?>
