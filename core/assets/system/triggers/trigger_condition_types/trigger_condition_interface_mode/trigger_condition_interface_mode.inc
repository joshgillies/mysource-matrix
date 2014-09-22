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
* $Id: trigger_condition_interface_mode.inc,v 1.4 2012/11/09 02:18:11 akarelia Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/general_occasional.inc';

/**
* Trigger Condition
*
* Checks whether the current user is in a particular interface
*
*
* @author  Basil Shkara <bshkara@squiz.net>
* @version $Revision: 1.4 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Trigger_Condition_Interface_Mode extends Trigger_Condition
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

		$satisfied = FALSE;
		$modes = $settings['modes'];
		// check the current interface against the list
		foreach ($modes as $mode) {
			switch ($mode) {
				case 'frontend':
					if (!SQ_IN_BACKEND && !SQ_IN_LIMBO && !SQ_IN_LOGIN && !SQ_IN_CRON) {
						$satisfied = TRUE;
					}
				break;

				case 'backend':
					if (SQ_IN_BACKEND) $satisfied = TRUE;
				break;

				case 'limbo':
					if (SQ_IN_LIMBO) $satisfied = TRUE;
				break;

				case 'login':
					if (SQ_IN_LOGIN) $satisfied = TRUE;
				break;

				case 'cron':
					if (SQ_IN_CRON) $satisfied = TRUE;
				break;
			}
		}

		return $satisfied;

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
		$selected = array_get_index($settings, 'modes', Array());

		ob_start();
		echo translate('User must be in ANY of the following interface modes:');

		if ($write_access) {
			$options = Array(
						'frontend',
						'backend',
						'limbo',
						'login',
						'cron',
					   );
			?>
				<ul class="sq-checkbox-list" style="margin-bottom: 0;">
					<?php
						foreach ($options as $idx => $option) {
							echo '<li>';
								echo ''.check_box($prefix.'['.$idx.']', $option, in_array($option, $selected)).'';
								echo ''.label(translate('condition_interface_mode_'.$option), $prefix.'['.$idx.']').'';
							echo '</li>';
						}
					?>
				</ul>
			<?php
		} else {
			echo '<ul style="margin-bottom: 0;">';
			foreach ($selected as $mode) {
				echo '<li><strong>'.translate('condition_interface_mode_'.$mode).'</strong></li>';
			}
			echo '</ul>';
		}
		$contents = ob_get_clean();
		return $contents;

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
		$settings['modes'] = $request_data;

		return FALSE;

	}//end processInterface()


}//end class

?>
