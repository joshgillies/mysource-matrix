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
* $Id: trigger_action_update_link.inc,v 1.6 2012/08/30 01:09:14 ewang Exp $
*
*/

require_once SQ_CORE_PACKAGE_PATH.'/system/triggers/trigger_action/trigger_action.inc';

/**
* Trigger_Action_Update_Link
*
* A trigger action that updates a link matching specified characteristics.
*
* @author  Mohamed Haidar <mhaidar@squiz.net>
* @version $Revision: 1.6 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Trigger_Action_Update_Link extends Trigger_Action
{


	/**
	* Execute this action
	* Returns true on success, or false if any error occurred
	*
	* The settings used by this action are in the form:<BR />
	* <PRE>
	* Array(
	* 		'linkid'				=> int,				// specific link id, all other settings ignored.
	*		'side_of_link'			=> string,			// side of the link the $state asset is on ('major' or 'minor')
	*       'link_type' 			=> int,				// SQ_LINK_* constants for link types
	* 		'type_code'				=> string,			// the type of asset that is linked (strict)
	*		'value'					=> string,			// link value
	*		'new_value'				=> string,			// new link value
	*		'new_link_type'			=> int,				// SQ_LINK_* constants for link types
	*       );
	* </PRE>
	*
	* @param array	$settings	the stored settings for this action
	* @param array	&$state		the state that describes the asset the action is executing on
	*
	* @return boolean
	* @access public
	*/
	public static function execute($settings, &$state)
	{
		$am = $GLOBALS['SQ_SYSTEM']->am;

		if (empty($state['asset'])) {
			// grab the asset if assetid is given, but not the asset.
			if (empty($state['assetid'])) {
				return FALSE;
			} else {
				$state['asset'] = $am->getAsset($state['assetid']);
			}
		}

		if (is_null($state['asset'])) return FALSE;

		$linkid = array_get_index($settings, 'linkid', '');

		$new_value = array_get_index($settings, 'new_value', NULL);
		$new_link_type = array_get_index($settings, 'new_link_type', NULL);

		$result = TRUE;
		if (!empty($linkid)){
			$result = $am->updateLink($linkid, $new_link_type, $new_value);
		} else {
			$type_code = array_get_index($settings, 'type_code', '');
			if (empty($type_code)) return FALSE;

			$link_type = array_get_index($settings, 'link_type', 0);
			if (empty($link_type)) return FALSE;

			$side_of_link = array_get_index($settings, 'side_of_link', '');
			if (!in_array($side_of_link, Array('minor', 'major'))) return FALSE;

			$value = array_get_index($settings, 'value', NULL);

			$links = $am->getLinks($state['asset']->id, $link_type, $type_code, TRUE, $side_of_link, $value, FALSE, FALSE);
			if (count($links) == 1){
				$linkid = $links[0]['linkid'];
				$result = $am->updateLink($linkid, $new_link_type, $new_value);
			} else {
				trigger_localised_error('TRIG0007', translate('More than one link found with the characteristics specified in \'Update Link\' Trigger Action.'), E_USER_NOTICE);
				return FALSE;
			}
		}

		return $result;

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
		require_once SQ_INCLUDE_PATH.'/general_occasional.inc';
		require_once SQ_LIB_PATH.'/html_form/html_form.inc';

		$link_type_names = get_link_type_names();

		$linkid = array_get_index($settings, 'linkid', '');
		$type_code = array_get_index($settings, 'type_code', '');
		$link_type = array_get_index($settings, 'link_type', key($link_type_names));
		$side_of_link = array_get_index($settings, 'side_of_link', 'major');
		$value = array_get_index($settings, 'value', '');

		$new_value = array_get_index($settings, 'new_value', '');
		$new_link_type = array_get_index($settings, 'new_link_type', key($link_type_names));

		ob_start();
			?>
			<div>
				<p class="sq-backend-section-subheading"><?php echo translate('Link Characteristics'); ?></p>

				<p><?php echo translate('You may specify the exact link that you want updated otherwise you need to specify the link characteristics as linked to the Broadcasting Asset. <br />
	This trigger action will be executed if exactly one link is found, otherwise it will fail. Leave <em>\'Link Value\'</em > blank to match any link value.');?></p>

				<?php
				echo 'Link Id: ';
				if ($write_access) {
					int_text_box($prefix.'[linkid]', $linkid, FALSE, '', NULL, NULL, '', FALSE, TRUE);
				} else {
					if (!empty($linkid)){
						echo $linkid;
					} else {
						echo '<em>Not specified</em>';
					}
				}
				echo '<br /><br /><em>OR</span></em><br /><br />';
				?>

				<table class="sq-backend-table with-bottom-margin">
					<col width="20%" />
					<col width="80%"/>
					<tr>
						<th style="vertical-align:middle;"><?php echo translate('Link Type'); ?>:</th>

						<td><?php
						if ($write_access) {
							combo_box($prefix.'[link_type]',  $link_type_names, FALSE, $link_type);
						} else {
							echo $link_type_names[$link_type];
						}
						?></td>
					</tr>
					<tr>
						<th style="vertical-align:middle;"><?php echo translate('Side of Link'); ?>:</th>

						<td><?php
						$side_of_link_options = Array('major' => 'Parent', 'minor' => 'Child');
						if ($write_access) {
							combo_box($prefix.'[side_of_link]', $side_of_link_options, FALSE, $side_of_link);
						} else {
							echo $side_of_link_options[$side_of_link];
						}
						?></td>
					</tr>
					<tr>
						<th style="vertical-align:middle;"><?php echo translate('Link Value'); ?>:</th>

						<td><?php
						if ($write_access) {
							text_box($prefix.'[value]', $value);
						} else {
							echo $value;
						}
						?></td>
					</tr>
					<tr>
						<th style="vertical-align:middle;"><?php echo translate('Asset Type'); ?>:</th>

						<td><?php
						if ($write_access) {
							asset_type_chooser($prefix.'[type_code]', FALSE, $type_code, TRUE);
						} else {
							echo ucwords(str_replace('_', ' ',$type_code));
						}
						?></td>
					</tr>
				</table>
			</div>
			<div>
				<p class="sq-backend-section-subheading"><?php echo translate('New Link Characteristics'); ?></p>

				<p><?php echo translate('<strong>Note:</strong> You cannot change a link from being a significant link to a non-significant link or vice versa.');?></p>

				<table class="sq-backend-table">
					<col width="20%" />
					<col width="80%"/>
					<tr>
						<th style="vertical-align:middle;"><?php echo translate('Link Type'); ?>:</th>

						<td><?php
						if ($write_access) {
							combo_box($prefix.'[new_link_type]',  $link_type_names, FALSE, $new_link_type);
						} else {
							echo $link_type_names[$new_link_type];
						}
						?></td>
					</tr>
					<tr>
						<th style="vertical-align:middle;"><?php echo translate('Link Value'); ?>:</th>

						<td><?php
						if ($write_access) {
							text_box($prefix.'[new_value]', $new_value);
						} else {
							echo $new_value;
						}
						?></td>
					</tr>
				</table>
			</div>
			<?php

			$output = ob_get_contents();
		ob_end_clean();

		return $output;

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
	public static function processInterface(&$settings, $request_data)
	{
		$settings['linkid'] = array_get_index($request_data, 'linkid', '');
		$settings['side_of_link'] = array_get_index($request_data, 'side_of_link', '');
		$settings['link_type'] = array_get_index($request_data, 'link_type', 0);
		$settings['link_type'] = (int)$settings['link_type'];

		$settings['type_code'] = array_get_index($request_data, 'type_code', '');

		// if we have linkid then we won't need the type_code
		if (empty($settings['linkid']) && empty($settings['type_code'])) return 'You must specify an Asset Type';

		$settings['value'] = array_get_index($request_data, 'value', NULL);
		$settings['value'] = trim($settings['value']);
		if (empty($settings['value'])) $settings['value'] = NULL;

		$settings['new_value'] = array_get_index($request_data, 'new_value', NULL);
		$settings['new_link_type'] = array_get_index($request_data, 'new_link_type', NULL);
		if (!is_null($settings['new_link_type'])) $settings['new_link_type'] = (int)$settings['new_link_type'];

		return FALSE;

	}//end processInterface()


	/**
	* Get the list of locks that the action needs acquire before executing
	*
	* @param array	$settings	settings that this condition saves in processInterface()
	* @param array	&$state		the state that describes the asset the action is executing on
	*
	* @return array
	* @access public
	*/
	public static function getLocks($settings, &$state)
	{
		return Array($state['assetid'] => Array('links'));

	}//end getLocks()


}//end class

?>
