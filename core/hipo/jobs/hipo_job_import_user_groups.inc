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
* $Id: hipo_job_import_user_groups.inc,v 1.1.2.1 2008/08/19 06:49:57 mbrydon Exp $
*
*/

require_once SQ_SYSTEM_ROOT.'/core/hipo/hipo_job.inc';

/**
* Highly Intensive Processing Object (HIPO) to import User Groups
* from a CSV file containing group names
*
* Example file contents:
*
*   Group 1, Group 2
*   Group 1, Group 2, Group 3
*   Group 4
*   Group 4, Group 5
*   Group 4, Group 5, Group 6
*   Group 4, Group 7
*
* Should cause the following User Group structure to be created:
*
*   [Existing root node]
*   |
*   |-- Group 1
*   |   |
*   |   |-- Group 2
*   |       |
*   |       |-- Group 3
*   |
*   |-- Group 4
*       |
*       |-- Group 5
*       |   |
*       |   |-- Group 6
*       |
*       | -- Group 7
*
* Initial Running Vars :
*    parent_id			-	the ID of asset under which groups are to be added
*    group_csv_filename -	a CSV file containing User Group data
*
* @author  Mark Brydon <mbrydon@squiz.net>
* @version $Revision: 1.1.2.1 $
* @package MySource_Matrix
* @subpackage HIPO
*/
class HIPO_Job_Import_User_Groups extends HIPO_Job
{


	/**
	* Constructor
	*
	* @param string	$code_name	a unique codename the HIPO
	*/
	function HIPO_Job_Import_User_Groups($code_name='')
	{
		$this->uses_trans = TRUE;
		$this->HIPO_Job($code_name);

	}//end constructor


	/**
	* Returns a unique codename the HIPO can use to ensure it is not being run twice
	*
	* @return string
	* @access public
	*/
	function getCodeName()
	{
		return parent::getCodeName().'-'.$this->_running_vars['parent_id'];

	}//end getCodeName()


	/**
	* Returns the (localised) HIPO name
	*
	* @return string
	* @access public
	*/
	function getHipoName()
	{
		return translate('hipo_name_import_user_groups');

	}//end getHipoName()


	/**
	* Returns the steps in this hipo, possibly with localised step names
	* and messages
	*
	* @return string
	* @access public
	*/
	function getInitialStepData()
	{
		return Array(
				Array(
					'name'			=> translate('hipo_import_user_groups_starting_process'),
					'function_call'	=> Array(
										'process_function'	=> 'processStart',
									   ),
					'running_mode'	=> 'server',
					'auto_step'		=> TRUE,
					'allow_cancel'	=> FALSE,
					'percent_done'	=> 0,
					'complete'		=> FALSE,
					'message'		=> translate('hipo_import_user_groups_setting_up_env'),
				),
				Array(
					'name'			=> translate('hipo_import_user_groups_parsing_csv'),
					'function_call'	=> Array(
										'process_function'	=> 'processImportUserGroups',
									   ),
					'running_mode'	=> 'server',
					'skip_step'		=> FALSE,
					'auto_step'		=> TRUE,
					'allow_cancel'	=> FALSE,
					'percent_done'	=> 0,
					'complete'		=> FALSE,
					'message'		=> '',
				),
			   );

	}//end getInitialStepData()


	/**
	* Performs the duties of this hipo, without showing any output on the frontend.
	*
	* @return boolean
	* @access public
	*/
	function freestyle()
	{
		$step_data = Array();
		$step_data['complete'] = FALSE;
		while (!$step_data['complete']) {
			if (!$this->processImportUserGroups($step_data, '')) {
				return FALSE;
			}
		}
		return TRUE;

	}//end freestyle()


	/**
	* Prepares the running vars and gets the children of the asset in question.
	*
	* @return boolean
	* @access public
	*/
	function prepare()
	{
		// Ensure that a Parent Root Node has been provided for the import
		if (is_null($this->_running_vars['parent_id'])) {
			trigger_localised_error('HIPO0061', E_USER_WARNING);
			return FALSE;
		}

		// Ensure that a CSV Group Import file is available
		if (is_null($this->_running_vars['group_csv_filename'])) {
			trigger_localised_error('HIPO0061', E_USER_WARNING);
			return FALSE;
		}

		if (!file_exists($this->_running_vars['group_csv_filename'])) {
			trigger_localised_error('HIPO0061', E_USER_WARNING);
			return FALSE;
		}

		// Ensure that the supplied Parent Root Node is valid
		$root_node =& $GLOBALS['SQ_SYSTEM']->am->getAsset($this->_running_vars['parent_id']);
		if (is_null($root_node)) {
			trigger_localised_error('HIPO0062', E_USER_WARNING, $this->_running_vars['parent_id']);
			return FALSE;
		}

		return parent::prepare();

	}//end prepare()


	/**
	* Dummy function that runs an empty step, so that there is no empty screen
	* before the end of the first real step
	*
	* @param array	&$step_data	step data
	* @param string	$prefix		unique prefix
	*
	* @return boolean
	* @access public
	*/
	function processStart(&$step_data, $prefix)
	{
		$step_data['percent_done'] = 100;
		$step_data['complete'] = TRUE;
		return TRUE;

	}//end processStart()


	/**
	* Import new User Groups from the supplied CSV file
	*
	* @param array	&$step_data	step data
	* @param string	$prefix		unique prefix
	*
	* @return boolean
	* @access public
	*/
	function processImportUserGroups(&$step_data, $prefix)
	{

		$block_size = 1024;
		$lines_per_step = 4;

		$GLOBALS['SQ_SYSTEM']->am->includeAsset('user_group');

		if (!isset($step_data['file_pointer_pos'])) {
			// assume this is the first step because we must read something on each step
			$step_data['file_pointer_pos'] = 0;
			$step_data['remainder'] = '';
			$step_data['known_group_structure'] = Array();
		}

		$import_file = $this->_running_vars['group_csv_filename'];
		if (!($fd = fopen($import_file, 'r'))) {
			trigger_localised_error('CORE0257', E_USER_WARNING, $import_file);
			return FALSE;
			die();
		}

		$fstat = fstat($fd);
		$file_size = $fstat['size'];

		if (isset($step_data['file_pointer_pos'])) {
			// Seek into the file past what we have already processed
			fseek($fd, $step_data['file_pointer_pos']);
		}

		$step_data['complete'] = FALSE;

		$lines = 0;
		while ((($group = fgetcsv($fd, $block_size, ',')) !== FALSE) && ($lines < $lines_per_step)) {
			$num_fields = count($group);

			// Skip any blank lines
			if ($num_fields > 0) {
				// Check for an existing User Group in our cached structure
				$cache =& $step_data['known_group_structure'];
				$current_parent_id = $this->_running_vars['parent_id'];

				for ($n=0; $n < $num_fields; $n++) {
					$group_name = trim($group[$n]);

					// We can't have empty names now, so go to the next line
					if (empty($group_name)) break;

					// Do we have an existing group in our cache?
					if (!isset($cache[$group_name])) {
						// If not, we should see if the group exists
						$existing_group_id = 0;
						$child_groups = $GLOBALS['SQ_SYSTEM']->am->getChildren($current_parent_id, 'user_group');

						foreach ($child_groups as $child_group_id => $child_group_type) {
							// Query the name of each unknown group
							$user_group =& $GLOBALS['SQ_SYSTEM']->am->getAsset($child_group_id);
							if ($user_group->attr('name') == $group_name) {
								$existing_group_id = $child_group_id;
								$GLOBALS['SQ_SYSTEM']->am->forgetAsset($user_group);
								break;
							}
							$GLOBALS['SQ_SYSTEM']->am->forgetAsset($user_group);
						}

						// If not found, create a User Group right here, right now
						if ($existing_group_id == 0) {
							$existing_group_id = $this->_createUserGroup($group_name, $current_parent_id);
						}

						// Cache the asset ID of the now-known User Group
						$cache[$group_name] = Array($existing_group_id);
					}

					// Keep digging
					$cache =& $cache[$group_name];
					$current_parent_id = $cache[0];
				}//end for
			}//end if

			$lines++;
		}//end while

		// Keep track of where we are in the file for next time
		$step_data['file_pointer_pos'] = ftell($fd);
		$step_data['percent_done'] = ($step_data['file_pointer_pos']/$file_size) * 100;

		if (feof($fd)) $step_data['complete'] = TRUE;

		$step_data['message']  = 'Processed '.sprintf('%01.2f',$step_data['file_pointer_pos']/1024).' of '.sprintf('%01.2f',$file_size/1024).' KB';
		fclose($fd);

		return TRUE;

	}//end processImportUserGroups()


	/**
	* Creates a User Group asset with the specified name under the root node supplied.
	* Returns the asset ID of the new User Group.
	*
	* @param string	$user_group_name	The name of the new User Group
	* @param int	$parent_asset_id	The Id of the asset under which to add the new User Group
	*
	* @return int
	* @access private
	*/
	function _createUserGroup($user_group_name, $parent_asset_id)
	{
		$user_group_id = 0;

		$parent_asset =& $GLOBALS['SQ_SYSTEM']->am->getAsset($parent_asset_id);
		if ($parent_asset->id) {
			$user_group =& new User_Group();
			$user_group->setAttrValue('name', $user_group_name);

			// Link the new asset under the parent asset
			$link_id = $this->_createLink($parent_asset, $user_group);

			$user_group_id = $user_group->id;
		}

		return $user_group_id;

	}//end _createUserGroup()


	/**
	* Creates a simple "TYPE 1" link between assets
	*
	* @param object	&$parent_asset	The parent asset
	* @param object	&$child_asset	The child asset
	* @param int	$link_type		The type of link to create
	*
	* @return int
	* @access private
	*/
	function _createLink(&$parent_asset, &$child_asset, $link_type=SQ_LINK_TYPE_1)
	{
		// Link the asset to the parent asset
		$link = Array(
					'asset'			=> $parent_asset,
					'link_type'		=> $link_type,
					'value'			=> '',
					'sort_order'	=> NULL,
					'is_dependant'	=> FALSE,
					'is_exclusive'	=> FALSE,
				);

		$link_id = $child_asset->create($link);

		return $link_id;

	}//end _createLink()


}//end class
?>