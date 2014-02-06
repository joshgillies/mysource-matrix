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
* $Id: trigger_action_set_thesaurus_terms.inc,v 1.8 2012/08/30 01:09:14 ewang Exp $
*
*/

require_once SQ_LIB_PATH.'/html_form/html_form.inc';
require_once SQ_FUDGE_PATH.'/general/file_system.inc';
require_once SQ_CORE_PACKAGE_PATH.'/system/triggers/trigger_action/trigger_action.inc';

/**
* Trigger Action Set Design
*
* A trigger action that import thesaurus XML file
*
*
* @author  Scott Kim <skim@squiz.net>
* @version $Revision: 1.8 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Trigger_Action_Set_Thesaurus_Terms extends Trigger_Action
{


	/**
	* Execute this action
	* Returns an array of data about what it did, or false on error
	*
	* The settings used by this action are in the form:
	* <PRE>
	* Array(
	*       'assetid'    => int     // the assetid of the design that will be applied to the asset stored in $state
	*       'file_path'  => string  // the path to XML file
	*       'append'     => boolean // append option (Not effective yet)
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
		if (empty($settings['file_path']) || !file_exists($settings['file_path'])) {
			// if no settings, fail
			return FALSE;
		}

		// check settings, append
		// NOTE: It does not have any effect in a hipo job at the moment
		if (empty($settings['append'])) {
			$append = FALSE;
		} else {
			$append = $settings['append'];
		}

		// state, design asset
		if (empty($state['asset'])) {
			// grab the asset if assetid is given, but not the asset.
			if (empty($state['assetid'])) {
				return FALSE;
			} else {
				$state['asset'] = $GLOBALS['SQ_SYSTEM']->am->getAsset($state['assetid']);
			}
		}

		if (is_null($state['asset'])) return FALSE;

		$asset =& $state['asset'];

		// copy over the new uploaded file
		if (is_readable($settings['file_path'])) {

			// a normal uploaded file
			if (!copy_file($settings['file_path'], $asset->getXmlFilePath())) {
				trigger_localised_error('CORE0235', E_USER_WARNING, $info['tmp_name'], $asset->getXmlFilePath());
				return FALSE;
			}
		}

		$hh = $GLOBALS['SQ_SYSTEM']->getHipoHerder();
		$running_vars = Array(
							'thesaurus_id'	=> $asset->id,
							'append'		=> $append,
							'file_path'		=> $settings['file_path'],
						);
		$errors = $hh->freestyleHipo('hipo_job_import_thesaurus_xml',  $running_vars);

		return Array(
				'assetid'	=> $state['asset']->id,
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
		$file_path = array_get_index($settings, 'file_path', '');
		$append = array_get_index($settings, 'append', FALSE);
		// paint the edit interface
		ob_start();
		?>
		<table class="sq-backend-table">
			<tr>
				<th><?php echo 'Path to XML File'; ?></th>
				<td><?php
					// level
					if ($write_access) {
						echo '<div class="sq-break-line-inputs">';
						echo text_box($prefix.'[file_path]', $file_path, '50');
						echo '&nbsp;&nbsp;';
						echo check_box($prefix.'[append]', '1', $append);
						echo label('Append?', $prefix.'[append]');
						echo '</div>';
					} else {
						echo $file_path.'&nbsp;';
						echo htmlspecialchars('(Append option is'.($append ? ' ' : ' not ').'selected)');
					}
				?>
				</td>
			</tr>
		</table>
		<?php

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
		$file_path = array_get_index($request_data, 'file_path', '');
		$file_path = trim($file_path);
		if ($file_path == '') {
			return 'file path is not specified.';
		}
		$settings['file_path'] = $file_path;

		$append = array_get_index($request_data, 'append', FALSE);
		$settings['append'] = $append;

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
										'attributes',
									   ),
			   );

	}//end getLocks()


}//end class

?>
