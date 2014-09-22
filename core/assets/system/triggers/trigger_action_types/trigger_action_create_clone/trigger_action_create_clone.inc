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
* $Id: trigger_action_create_clone.inc,v 1.23 2012/08/30 01:09:13 ewang Exp $
*
*/

require_once SQ_LIB_PATH.'/html_form/html_form.inc';
require_once SQ_INCLUDE_PATH.'/general_occasional.inc';

/**
* Trigger Action Create Clone
*
* A trigger action that creates a clone of the firing asset
*
*
* @author   Andrei Railean <arailean@squiz.net>
* @author   Robert Howard <rhoward@squiz.net>
* @version $Revision: 1.23 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Trigger_Action_Create_Clone extends Trigger_Action
{


	/**
	* Execute this action
	* Returns an array of data about what it did, or false on error
	*
	* The settings used by this action are in the form:
	* <PRE>
	* Array(
	*       'assetid'             => int,    // the assetid to link to
	*       'link_type'           => int,    // the type of link, eg. type_1, type_2, type_3 or notice
	*       'value'               => string, // optional, the value associated with a link
	*       'is_dependant'        => int,    // optional, whether the link is dependant
	*       'is_exclusive'        => int,    // optional, whether the link is exclusive
	*       'disallow_duplicates' => int,    // if '1', then clone will not be created if an asset
	*                                        // with the same name and link_type to the target exists.
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
		$clone_broadcaster   = array_get_index($settings, 'clone_broadcaster', 1);
		$link_to_broadcaster = array_get_index($settings, 'link_to_broadcaster', 0);

		if (($clone_broadcaster == 0) && (empty($settings['clone_assetid']) || !$GLOBALS['SQ_SYSTEM']->am->assetExists($settings['clone_assetid']))) {
			return FALSE;
		}

		if (($link_to_broadcaster == 0) && (empty($settings['assetid']) || !$GLOBALS['SQ_SYSTEM']->am->assetExists($settings['assetid']))) {
			return FALSE;
		}

		if (empty($settings['link_type'])) {
			return FALSE;
		}

		// check the optional settings, assign defaults
		$is_dependant        = array_get_index($settings, 'is_dependant', 0);
		$is_exclusive        = array_get_index($settings, 'is_exclusive', 0);
		$disallow_duplicates = array_get_index($settings, 'disallow_duplicates', 0);

		if (empty($state['asset'])) {
			// grab the asset if assetid is given, but not the asset.
			if (empty($state['assetid'])) {
				return FALSE;
			} else {
				$state['asset'] = $GLOBALS['SQ_SYSTEM']->am->getAsset($state['assetid']);
			}
		}

		$am = $GLOBALS['SQ_SYSTEM']->am;

		$major = NULL;
		if ($link_to_broadcaster == 1) {
			$major = $state['asset'];
		} else {
			$major = $am->getAsset($settings['assetid']);
		}

		$minor = NULL;
		if ($clone_broadcaster == 1) {
			$minor = $state['asset'];
		} else {
			$minor = $am->getAsset($settings['clone_assetid']);
		}

		if ($disallow_duplicates) {
			// get all the children of the target asset, check whether an asset
			// with the same name, short_name and type_code already exists
			$children = $am->getChildren($major->id);
			foreach ($children as $childid => $child_type) {
				$name = $am->getAssetInfo(Array($childid), Array(), TRUE, 'name');
				$short_name = $am->getAssetInfo(Array($childid), Array(), TRUE, 'short_name');
				$type_code = $am->getAssetInfo(Array($childid), Array(), TRUE, 'type_code');
				if ($name[$childid] == $minor->name && $short_name[$childid] == $minor->short_name && $type_code[$childid] == $minor->type()) {
					// a copy of the clone already exists
					return Array(
							'assetid'		=> $childid,
							'parentid'		=> $major->id,
							'link_type'		=> $settings['link_type'],
							'value'			=> $settings['value'],
							'is_dependant'	=> $is_dependant,
							'is_exclusive'	=> $is_exclusive,
						   );
				}
			}
		}

		// the new link
		$link = Array(
					'asset'			=> &$major,
					'link_type'		=> $settings['link_type'],
					'value'			=> $settings['value'],
					'sort_order'	=> NULL,
					'is_dependant'	=> $is_dependant,
					'is_exclusive'	=> $is_exclusive,
				);

		// old -> new asset id mapping
		$clone_map = Array();

		// Clone only the following atrributes. (tagging will be cloned by cloneLinks)
		$components = Array(
						'attributes',
                        'metadata_schemas',
                        'metadata',
                        'workflow',
                        'permissions',
                        'roles',
                      );

		$GLOBALS['SQ_SYSTEM']->setRunLevel(SQ_RUN_LEVEL_FORCED);
		$clone = $am->cloneAsset($minor, $link, $clone_map, $components);
		$GLOBALS['SQ_SYSTEM']->restoreRunLevel();

		if (is_null($clone)) {
			trigger_localised_error('CORE0138', translate('clone failed'), E_USER_WARNING)
			return FALSE;
		}

		$valid_paths = make_valid_web_paths($minor->getWebPaths());
		$good_paths = $GLOBALS['SQ_SYSTEM']->am->webPathsInUse($link['asset'], $valid_paths, $clone->id, TRUE);

		$clone->saveWebPaths($good_paths);
		$clone->remapAssetids($clone_map);

		return Array(
				'assetid'		=> $clone->id,
				'parentid'		=> $major->id,
				'link_type'		=> $settings['link_type'],
				'value'			=> $settings['value'],
				'is_dependant'	=> $is_dependant,
				'is_exclusive'	=> $is_exclusive,
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
		// check settings, set defaults if necessary
		$clone_assetid       = array_get_index($settings, 'clone_assetid', 0);
		$selected_assetid    = array_get_index($settings, 'assetid', 0);
		$selected_link_type  = array_get_index($settings, 'link_type', SQ_LINK_TYPE_1);
		$value               = array_get_index($settings, 'value', '');
		$is_dependant        = array_get_index($settings, 'is_dependant', 0);
		$is_exclusive        = array_get_index($settings, 'is_exclusive', 0);
		$disallow_duplicates = array_get_index($settings, 'disallow_duplicates', 0);
		$clone_broadcaster   = array_get_index($settings, 'clone_broadcaster', 1);
		$link_to_broadcaster = array_get_index($settings, 'link_to_broadcaster', 0);

		// check if the cloned asset is valid
		if ($clone_assetid && !$GLOBALS['SQ_SYSTEM']->am->assetExists($clone_assetid)) {
			$clone_assetid = 0;
		}

		// check if the selected asset is valid
		if ($selected_assetid && !$GLOBALS['SQ_SYSTEM']->am->assetExists($selected_assetid)) {
			$selected_assetid = 0;
		}

		$link_type_list = get_link_type_names();
		unset($link_type_list[SQ_LINK_NOTICE]);

		if (!$write_access) {
			$form_element_extras = 'disabled="disabled"';
		} else {
			$form_element_extras = '';
		}

		// begin buffering basic options
		ob_start();
			if ($write_access) {
				asset_finder($prefix.'[clone_assetid]', $clone_assetid);
			} else {
				if ($clone_assetid) {
					$clone_assetname = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo(Array($clone_assetid), Array(), TRUE, 'name');
					echo get_asset_tag_line($clone_assetid);
				} else {
					echo '<b>['.translate('No asset selected').']</b>';

				}
			}

			$basic_part_1 = ob_get_contents();
		ob_end_clean();

		ob_start();
			if ($write_access) {
				asset_finder($prefix.'[assetid]', $selected_assetid);
			} else {
				if ($selected_assetid) {
					$selected_assetname = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo(Array($selected_assetid), Array(), TRUE, 'name');
					echo get_asset_tag_line($selected_assetid);
				} else {
					text_box($prefix.'[assetid]', '', '', '', FALSE, $form_element_extras);
					echo '<b>['.translate('no parent selected').']</b>';

				}
			}

			$basic_part_2 = ob_get_contents();
		ob_end_clean();

		ob_start();
			combo_box($prefix.'[link_type]', $link_type_list, FALSE, $selected_link_type, NULL, $form_element_extras);
			$basic_part_3 = ob_get_contents();
		ob_end_clean();

		// begin buffering advanced options
		ob_start();
			text_box($prefix.'[value]', $value, '', '', FALSE, $form_element_extras);
			$advanced_part_1 = ob_get_contents();
		ob_end_clean();

		ob_start();
			$is_dependant_list = Array(
									0	=> translate('Non-dependently'),

									1	=> translate('Dependently'),

								 );
			combo_box($prefix.'[is_dependant]', $is_dependant_list, FALSE, $is_dependant, NULL, $form_element_extras);
			$advanced_part_2 = ob_get_contents();
		ob_end_clean();

		ob_start();
			$is_exclusive_list = Array(
									0	=> translate('Non-exclusively'),

									1	=> translate('Exclusively'),

								 );
			combo_box($prefix.'[is_exclusive]', $is_exclusive_list, FALSE, $is_exclusive, NULL, $form_element_extras);
			$advanced_part_3 = ob_get_contents();
		ob_end_clean();

		ob_start();
			check_box($prefix.'[disallow_duplicates]', '1', $disallow_duplicates, NULL, $form_element_extras);
			label(translate('Do not create if a clone with the same name and asset type already exists'), $prefix.'[disallow_duplicates]');

			$advanced_part_4 = ob_get_contents();
		ob_end_clean();

		ob_start();
			check_box($prefix.'[clone_broadcaster]', '1', $clone_broadcaster, NULL, $form_element_extras);
			label(translate('Clone broadcasting asset'), $prefix.'[clone_broadcaster]');

			$advanced_part_5 = ob_get_contents();
		ob_end_clean();

		ob_start();
			check_box($prefix.'[link_to_broadcaster]', '1', $link_to_broadcaster, NULL, $form_element_extras);
			label(translate('Link to broadcasting asset'), $prefix.'[link_to_broadcaster]');

			$advanced_part_6 = ob_get_contents();
		ob_end_clean();


		// now patch together the output
		$contents = sprintf(translate('Create a clone of asset %1$s and link it as child of asset %2$s using a %3$s link.'), $basic_part_1, $basic_part_2, $basic_part_3).

					'<br /><br />'.
					'<b>'.translate('Optional items for Advanced users').'</b><br />'.

					sprintf(translate('Use the link value %1$s and link the asset %2$s to the parent.<br /><br />%4$s<br />%5$s<br />%6$s'), $advanced_part_1, $advanced_part_2, $advanced_part_3, $advanced_part_4, $advanced_part_5, $advanced_part_6);


		return $contents;

	}//end getInterface()


	/**
	* Handle the conversion of interface to settings
	* together with settings it is expected to populate the hash object
	*
	* @param array	&$settings		a container for any data the action might want to save
	* @param string	$request_data	array of data corresponding to this action as specified
	*								by the $prefix in getInterface.
	*
	* @return boolean
	* @access public
	*/
	public static function processInterface(&$settings, $request_data)
	{
		$link_type_list = get_link_type_names();

		$clone_broadcaster   = array_get_index($request_data, 'clone_broadcaster', 0);
		$link_to_broadcaster = array_get_index($request_data, 'link_to_broadcaster', 0);

		// make sure the clone assetid isn't blank if "Clone broadcasting asset" is not selected
		if (($clone_broadcaster == 0) && empty($request_data['clone_assetid']['assetid'])) {
			return translate('The asset to clone is missing and cloning broadcasting asset is not selected');

		}

		// make sure the assetid isn't blank if "Link to broadcasting asset" is not selected
		if (($link_to_broadcaster == 0) && empty($request_data['assetid']['assetid'])) {
			return translate('Parent to clone under is missing and linking to broadcasting asset is not selected');

		}

		if (empty($request_data['link_type']) || !isset($link_type_list[$request_data['link_type']])) {
			return translate('Link type is missing');

		}

		$settings['clone_assetid']			= $request_data['clone_assetid']['assetid'];
		$settings['assetid']				= $request_data['assetid']['assetid'];
		$settings['link_type']				= $request_data['link_type'];
		$settings['value']					= array_get_index($request_data, 'value', '');
		$settings['is_dependant']			= array_get_index($request_data, 'is_dependant', 0);
		$settings['is_exclusive']			= array_get_index($request_data, 'is_exclusive', 0);
		$settings['disallow_duplicates']	= array_get_index($request_data, 'disallow_duplicates', 0);
		$settings['clone_broadcaster']		= $clone_broadcaster;
		$settings['link_to_broadcaster']	= $link_to_broadcaster;

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
				$state['assetid']	=> Array('all'),
			   );

	}//end getLocks()


}//end class

?>
