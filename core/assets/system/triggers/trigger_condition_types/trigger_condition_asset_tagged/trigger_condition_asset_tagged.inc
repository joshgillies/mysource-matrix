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
* $Id: trigger_condition_asset_tagged.inc,v 1.7 2012/08/30 01:09:14 ewang Exp $
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
* @version $Revision: 1.7 $
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
	* @param array	$settings	the stored settings for this condition
	* @param array	&$state		the state that describes the asset the condition is executing on
	*
	* @return boolean
	* @access public
	*/
	public static function evaluate($settings, &$state)
	{
		$tag_manager = $GLOBALS['SQ_SYSTEM']->getTagManager();

		$assetid	= $state['assetid']; // event broadcaster
		$tagids		= array_get_index($settings, 'tagids');
		$is_strict	= array_get_index($settings, 'is_strict', FALSE);

		if (empty($tagids)) {
			trigger_localised_error('TRIG0004', translate('No tag is specified in trigger condition: \'Asset is tagged\''), E_USER_WARNING)
			return FALSE;
		}
		return $tag_manager->isAssetTaggedWith($assetid, $tagids, $is_strict);

	}//end evaluate()


	/**
	* Get the HTML editing interface for this condition. Returns the string instead of printing it
	*
	* @param array		$settings		settings that this condition save in processInterface()
	* @param string		$prefix			unique prefix
	* @param boolean	$write_access	write access flag
	*
	* @return boolean
	* @access public
	*/
	public static function getInterface($settings, $prefix, $write_access=FALSE)
	{
		// check settings, set defaults if necessary
		$tagids		= array_get_index($settings, 'tagids', Array());
		$is_strict	= array_get_index($settings, 'is_strict', FALSE);


		ob_start();
			$strict_options[TRUE] = translate('All');

			$strict_options[FALSE] = translate('Some');


			if ($write_access) {
				combo_box($prefix.'[is_strict]', $strict_options, FALSE, $is_strict);
			} else {
				echo '<strong>'.$strict_options[$is_strict].'</strong>';
			}

			echo ' '.translate('tags must be assigned');

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
	* @param array	&$settings		a container for any data the condition might want to save
	* @param array	$request_data	array of data corresponding to this condition as specified by the $prefix in the getInterface.
	*
	* @return boolean
	* @access public
	*/
	public static function processInterface(&$settings, $request_data)
	{
		$tagids = array_get_index($request_data, 'tagids', Array());
		$settings['is_strict'] = (boolean) array_get_index($request_data, 'is_strict', FALSE);

		foreach ($tagids as $tag) {
			if (empty($tag['assetid'])) continue;
			$settings['tagids'][$tag['assetid']] = $tag['assetid'];
		}

		return FALSE;

	}//end processInterface()


}//end class

?>
