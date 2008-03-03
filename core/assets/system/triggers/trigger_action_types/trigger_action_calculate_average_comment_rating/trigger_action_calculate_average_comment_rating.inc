<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix Module file is Copyright (c) Squiz Pty Ltd    |
* | ACN 084 670 600                                                    |
* +--------------------------------------------------------------------+
* | IMPORTANT: This Module is not available under an open source       |
* | license and consequently distribution of this and any other files  |
* | that comprise this Module is prohibited. You may only use this     |
* | Module if you have the written consent of Squiz.                   |
* +--------------------------------------------------------------------+
*
* $Id: trigger_action_calculate_average_comment_rating.inc,v 1.1.2.3 2008/03/03 22:36:49 lwright Exp $
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
* @version $Revision: 1.1.2.3 $
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
	*		'fieldid'		=> int		// the assetid of the metadata field which stores the average rating value
	*		'parentdata'	=> array	// key contains assetid of parent asset and value contains average rating
	* );
	* </PRE>
	*
	* @param array	$settings	the stored settings for this action
	* @param array	&$state		the state that describes the asset the action is executing on
	*
	* @return mixed array|boolean
	* @access public
	*/
	function execute($settings, &$state)
	{
		// check required settings
		if (empty($settings['fieldid']['assetid'])) {
			return FALSE;
		} else {
			$fieldid = $settings['fieldid']['assetid'];
		}

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
					$numofratings++;
					$asset =& $GLOBALS['SQ_SYSTEM']->am->getAsset($link_info['minorid']);
					$rating = $asset->attr('rating');
					// means this rating hasn't been set, i.e. default value
					if ($rating === '-1')
						continue;
					$sumofratings += $rating;
				}
				// set the average for this parent (percentage)
				$avgofratings = (int) ($sumofratings/$numofratings);
				// returned array data
				$parentdata[$parentid] = $avgofratings;
				$mm =& $GLOBALS['SQ_SYSTEM']->getMetadataManager();
				$metadata = Array(
								$fieldid	=> Array(
												'value'	=> $avgofratings,
											   ),
							);

				// get the parent schema of this metadata field
				$schemaid = $GLOBALS['SQ_SYSTEM']->am->getParents($fieldid, 'metadata_schema');
				// get the schemas that apply to this asset
				$schemas = $mm->getSchemas($parentid, TRUE);

				if (array_intersect($schemas, array_keys($schemaid)) !== Array()) {
					$GLOBALS['SQ_SYSTEM']->setRunLevel(SQ_RUN_LEVEL_FORCED);
					$success = $mm->setMetadata($parentid, $metadata);
					if ($success) $mm->generateContentFile($parentid);
					$GLOBALS['SQ_SYSTEM']->restoreRunLevel();
				}
			}//end foreach
		}//end if

		if ($success) {
			return Array(
					'fieldid'		=> $fieldid,
					'parentdata'	=> $parentdata,
				   );
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
	function getInterface($settings, $prefix, $write_access=FALSE)
	{
		$fieldid = array_get_index($settings, 'fieldid', '');
		ob_start();

			echo translate('trigger_avg_comment_rating_metadata_field');
			if ($write_access) {
				$type_code = Array(
								'metadata_field_text'	=> 'I',
							 );
				echo asset_finder($prefix.'[fieldid]', $fieldid['assetid'], $type_code);
			} else {
				if (empty($fieldid['assetid'])) {
					echo '<b>['.translate('no_asset_selected').']</b>';
				} else {
					echo get_asset_tag_line($fieldid['assetid']);
				}
			}

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
	function processInterface(&$settings, $request_data)
	{
		$fieldid = array_get_index($request_data, 'fieldid', '');
		if (empty($fieldid)) return FALSE;
		$settings['fieldid'] = $fieldid;

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
	function getLocks($settings, &$state)
	{
		return Array();

	}//end getLocks()


}//end class
