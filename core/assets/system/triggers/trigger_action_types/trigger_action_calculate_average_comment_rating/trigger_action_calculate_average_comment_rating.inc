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
* $Id: trigger_action_calculate_average_comment_rating.inc,v 1.13 2012/08/30 01:09:12 ewang Exp $
*
*/

require_once SQ_FUDGE_PATH.'/general/file_system.inc';
require_once SQ_CORE_PACKAGE_PATH.'/system/triggers/trigger_action/trigger_action.inc';


/**
* Trigger Action Calculate Average Comment Rating
*
* A trigger action that calculates the average comment rating and then sets this value
* to a specified metadata field on the parent asset of the comments.
*
*
* @author  Basil Shkara <bshkara@squiz.net>
* @version $Revision: 1.13 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Trigger_Action_Calculate_Average_Comment_Rating extends Trigger_Action
{


	/**
	* Execute this action
	* Returns an array of data about what it did, or false on error
	*
	* The settings used by this action are in the form:<BR />
	* <PRE>
	* Array(
	*		'fieldid'	=> string	// the assetid of the metadata field which stores the average rating value
	*		'totalid'	=> string	// key contains assetid of parent asset and value contains average rating
	* );
	* </PRE>
	*
	* The "parentdata" return includes the average ratings of all assets this
	* action processed. The remaining outputs mirror the inputs.
	*
	* @param array	$settings	the stored settings for this action
	* @param array	&$state		the state that describes the asset the action is executing on
	*
	* @return mixed array|boolean
	* @access public
	*/
	public static function execute($settings, &$state)
	{
		// check required settings
		if (empty($settings['fieldid']['assetid'])) {
			return FALSE;
		} else {
			$fieldid = $settings['fieldid']['assetid'];
		}
		if (empty($settings['totalid']['assetid'])) {
			return FALSE;
		} else {
			$totalid = $settings['totalid']['assetid'];
		}

		// Define $success here so we know we have success, if the trigger fails it should report to us further down
		$success = 1;

		// set the metadata value on all immediate parents if they have the schema applied
		if (!empty($state['immediate_parents'])) {
			foreach ($state['immediate_parents'] as $parentid => $type_code) {
				// get all comment ratings and calculate average
				$comment_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($parentid, SQ_SC_LINK_ALL, 'comment');
				$numofratings = 0;
				$sumofratings = 0;
				$assetdeleted = 0;
				// check if we are deleting an asset
				if ($state['event']['name'] === 'trigger_event_before_asset_deleted') {
					// remove this asset from our list of links to process
					$assetdeleted = $state['assetid'];
				}

				foreach ($comment_links as $link_info) {
					// skip asset if we are deleting it
					if ($link_info['minorid'] === $assetdeleted) continue;
					$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($link_info['minorid']);
					$rating = $asset->attr('rating');
					// means this rating hasn't been set, i.e. default value
					if ($rating === '-1') continue;
					// include the asset to total assets only if we are getting the rating from it
					// unrated comment should be kept at distance
					$numofratings++;
					$sumofratings += $rating;
				}

				// check first $numofratings is not zero because you cannot divide by zero (ie. no items found)
				if ($numofratings > 0) {
					// set the average for this parent (percentage)
					$avgofratings = (int) ($sumofratings/$numofratings);
					// returned array data
					$parentdata[$parentid] = $avgofratings;
				} else {
					$avgofratings = 0;
					$numofratings = 0;
					$parentdata[$parentid] = 0;
				}

				// Update the metadata fields
				$mm =& $GLOBALS['SQ_SYSTEM']->getMetadataManager();
				$metadata = Array(
								$fieldid	=> Array(
												0	=> Array(
														'value'	=> $avgofratings,
													   ),
											   ),
								$totalid	=> Array(
												0	=> Array(
														'value'	=> $numofratings,
													   ),
											   ),
							);

				// get the parent schema of this metadata field
				$schemaid = $GLOBALS['SQ_SYSTEM']->am->getParents($fieldid, 'metadata_schema');
				// get the schemas that apply to this asset
				$schemas = $mm->getSchemas($parentid, TRUE);

				if (array_intersect($schemas, array_keys($schemaid)) !== Array()) {
					$GLOBALS['SQ_SYSTEM']->setRunLevel(SQ_RUN_LEVEL_FORCED);
					$success = $mm->setMetadata($parentid, $metadata);
					if ($success) {
						$fieldid_field = $GLOBALS['SQ_SYSTEM']->am->getAsset($fieldid);
						$totalid_field = $GLOBALS['SQ_SYSTEM']->am->getAsset($totalid);
						// If both of these two fields are contextable, regenerate the metadata file in the current context only;
						// otherwise, regenerate all metadata files
						if ($fieldid_field->attr('is_contextable') && $totalid_field->attr('is_contextable')) {
							$mm->generateContentFile($parentid);
						} else {
							$all_contexts = $GLOBALS['SQ_SYSTEM']->getAllContexts();
							foreach ($all_contexts as $contextid => $context_data) {
								$mm->generateContentFile($parentid, FALSE, $contextid);
							}
						}
					}
					$GLOBALS['SQ_SYSTEM']->restoreRunLevel();
				}

				$success_results = Array(
									'fieldid'		=> $fieldid,
									'totalid'		=> $totalid,
									'parentdata'	=> $parentdata,
								   );
			}//end foreach
		}//end if

		if ($success) {
			return $success_results;
		} else {
			return FALSE;
		}

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
		$fieldid = array_get_index($settings, 'fieldid', '0');
		$totalid = array_get_index($settings, 'totalid', '0');
		ob_start();

			// The average rating field store
			echo translate('trigger_avg_comment_rating_metadata_field');
			if ($write_access) {
				$type_code = Array(
								'metadata_field_text'	=> 'I',
							 );
				if (isset($fieldid['assetid'])) {
					$field_selected_asset = $fieldid['assetid'];
				} else {
					$field_selected_asset = 0;
				}
				echo asset_finder($prefix.'[fieldid]', $field_selected_asset, $type_code);
			} else {
				if (empty($fieldid['assetid'])) {
					echo '<b>['.translate('no_asset_selected').']</b>';
				} else {
					echo get_asset_tag_line($fieldid['assetid']);
				}
			}
			echo '<br />';
			echo '<br />';

			// The total comments field store
			echo translate('trigger_total_comment_rating_metadata_field');
			if ($write_access) {
				$type_code = Array(
								'metadata_field_text'	=> 'I',
							 );
				if (isset($totalid['assetid'])) {
					$total_selected_asset = $totalid['assetid'];
				} else {
					$total_selected_asset = 0;
				}
				echo asset_finder($prefix.'[totalid]', $total_selected_asset, $type_code);
			} else {
				if (empty($totalid['assetid'])) {
					echo '<b>['.translate('no_asset_selected').']</b>';
				} else {
					echo get_asset_tag_line($totalid['assetid']);
				}
			}
			echo '<br />';

		return ob_get_clean();

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
		// Store the average field
		$fieldid = array_get_index($request_data, 'fieldid', '');
		if (empty($fieldid)) return FALSE;
		$settings['fieldid'] = $fieldid;

		// Store the total field
		$totalid = array_get_index($request_data, 'totalid', '');
		if (empty($totalid)) return FALSE;
		$settings['totalid'] = $totalid;

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
		return Array();

	}//end getLocks()


}//end class
