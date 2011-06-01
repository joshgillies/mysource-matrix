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
* $Id: trigger_condition_url_matches.inc,v 1.10 2007/12/11 02:57:28 rong Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/general_occasional.inc';

/**
* Trigger Condition
*
* Checks whether the current URL matches pattern
*
*
* @author  Rayn Ong <rong@squiz.net>
* @version $Revision: 1.10 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Trigger_Condition_URL_Matches extends Trigger_Condition
{


	/**
	* Evaluate this condition
	*
	* The settings used by this condition are in the form:
	* <PRE>
	* Array(
	*       'pattern' => string      // the pattern of the url to match
	*       'match'   => boolean     // either match or do not match the pattern
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
		// grab the data we need to check the condition, if it's not already cached
		// note that new state is modified and new data is available to other conditions
		if (empty($state['asset'])) {
			// grab the asset if assetid is given, but not the asset
			if (empty($state['assetid'])) {
				return FALSE;
			} else {
				$state['asset'] = $GLOBALS['SQ_SYSTEM']->am->getAsset($state['assetid']);
			}
		}

		if (empty($settings['pattern'])) {
			// if no pattern settings, fail
			return FALSE;
		}

		// see if the pattern occurs in the current url
		if (isset($_SERVER['REQUEST_URI'])) {
			$matches = strpos($_SERVER['REQUEST_URI'], $settings['pattern']);
			// check the condition
			if ($settings['match'] && $matches) {
				return TRUE;
			} else if (!$settings['match'] && !$matches) {
				return TRUE;
			}
		}
		return FALSE;

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
		if (!$write_access) {
			$form_element_extras = 'disabled="disabled"';
		} else {
			$form_element_extras = '';
		}

		$pattern = (isset($settings['pattern'])) ? $settings['pattern'] : '';
		$match = (isset($settings['match'])) ? $settings['match'] : FALSE;

		// begin buffering basic options
		ob_start();
			if ($write_access) {
				text_box($prefix.'[pattern]', $pattern, '100', '', FALSE, $form_element_extras);
				echo '<br>';

				radio_button($prefix.'[match]', '1', $match, NULL,  $form_element_extras);
				echo ' Must be in URL<br>';
				radio_button($prefix.'[match]', '0', !$match, NULL, $form_element_extras);
				echo ' Must not be in URL<br>';
				echo 'Note: this condition only checks the query string ($_SERVER["REQUEST_URI"]) part of the URL';
			} else {
				$match_str = ($match) ? 'must be in URL' : 'must not be in URL';
				echo "\"$pattern\" $match_str .";
			}
			$basic_part_1 = ob_get_contents();
		ob_end_clean();

		return ' URL pattern '.$basic_part_1;

	}//end getInterface()


	/**
	* Function that handles the conversion of interface to settings
	*
	* @param array	&$settings		a container for any data the condition might want to save
	* @param array	$request_data	array of data corresponding to this condition as specified by the $prefix in the getInterface.
	*
	* @return mixed boolean|string
	* @access public
	*/
	public static function processInterface(&$settings, $request_data)
	{
		$pattern = array_get_index($request_data, 'pattern', FALSE);
		if (!$pattern) {
			return 'No URL pattern entered';
		} else {
			$settings['pattern'] = $pattern;
			// store if we want to match pattern or not
			$settings['match'] = array_get_index($request_data, 'match', FALSE);
			return FALSE;
		}

	}//end processInterface()


}//end class

?>
