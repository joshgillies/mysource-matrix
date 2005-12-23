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
* $Id: trigger_condition_asset_tagged.inc,v 1.1 2005/12/23 04:33:44 arailean Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/general_occasional.inc';

/**
* Trigger_Condition_Asset_Tagged
*
* Checks whether a link from the firing asset to an asset of a particular type exists
*
*
* @author   Andrei Railean <arailean@squiz.net>
* @version $Revision: 1.1 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Trigger_Condition_Asset_Tagged extends Trigger_Condition
{


	/**
	* Evaluate this condition
	*
	* The settings used by this condition are in the form:
	* <PRE>
	* Array(
	*       'tagid'        => string,
	*       'is_strict'    => boolean,
	*       );
	* </PRE>
	*
	* @param array(string=>mixed)   $settings   the stored settings for this condition
	* @param array(string=>mixed)   &$state     the state that describes the asset the condition is executing on
	*
	* @return boolean
	* @access public
	*/
	function evaluate($settings, &$state)
	{
		$tag_manager = &$GLOBALS['SQ_SYSTEM']->getTagManager();

		$assetid	= $state['assetid']; // event broadcaster
		$tagids		= array_get_index($settings, 'tagids');
		$is_strict	= array_get_index($settings, 'is_strict', false);

		return $tag_manager->isAssetTaggedWith($assetid, $tagids, $is_strict);

	}//end evaluate()


	/**
	* Get the HTML editing interface for this condition. Returns the string instead of printing it
	*
	* @param Array      $settings  settings that this condition save in processInterface()
	* @param string     $prefix    unique prefix
	* @param boolean    $write_access   write access flag
	*
	* @access public
	* @return boolean
	*/
	function getInterface($settings, $prefix, $write_access=false)
	{
		// check settings, set defaults if necessary
		$tagids		= array_get_index($settings, 'tagids', Array());
		$is_strict	= array_get_index($settings, 'is_strict', false);


		ob_start();
			$strict_options[TRUE] = translate('all');
			$strict_options[FALSE] = translate('some');

			if ($write_access) {
				combo_box($prefix.'[is_strict]', $strict_options, false, $is_strict);
			} else {
				echo '<strong>'.$strict_options[$is_strict].'</strong>';
			}

			echo ' '.translate('tags_must_be_assigned');
			echo '<br />';
			echo '<br />';

			$interface = ob_get_contents();
		ob_end_clean();

		ob_start();

			if ($write_access) {
				multiple_asset_finder($prefix.'[tagids]', $tagids, Array('thesaurus_term'=>'D'));
			} else {
				foreach ($tagids as $tagid) {
					echo get_asset_tag_line($tagid).'&nbsp; &nbsp;';
				}
			}

			$interface .= ob_get_contents();
		ob_end_clean();


		return $interface;

	}//end getInterface()


	/**
	* Function that handles the conversion of interface to settings
	*
	* @param Array          $settings       a container for any data the condition might want to save
	* @param array          $request_data   array of data corresponding to this condition as specified by the $prefix in the getInterface.
	*
	* @access public
	* @return boolean
	*/
	function processInterface(&$settings, $request_data)
	{
		$tagids = array_get_index($request_data, 'tagids', Array());
		$settings['is_strict'] = (boolean) array_get_index($request_data, 'is_strict', false);

		foreach ($tagids as $tag) {
			if (empty($tag['assetid'])) continue;
			$settings['tagids'][$tag['assetid']] = $tag['assetid'];
		}

		return false;

	}//end processInterface()


}//end class

?>
