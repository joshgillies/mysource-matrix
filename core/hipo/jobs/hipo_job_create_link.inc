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
* $Id: hipo_job_create_link.inc,v 1.7 2003/12/16 12:13:57 brobertson Exp $
* $Name: not supported by cvs2svn $
*/

require_once SQ_SYSTEM_ROOT.'/core/hipo/hipo_job.inc';

/**
* Highly Intensive Processing Object (HIPO) to edit asset permissions
*
* <pre>
* Initial Running Vars :
*    int    assetid             -  the assetid with whose children we are going to attempt to lock
*    Array  permission_changes  -  the values that we are trying to set for our children, in the form :
*                                  $changes = Array([perm] => Array (
*                                                                       Array(
*                                                                           'access'          => $access,
*                                                                           'userid'          => $userid,
*                                                                           'previous_access' => null,
*                                                                      ),
*                                                   ),
* </pre>
*
* @author  Marc McIntyre <mmcintyre@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix
* @subpackage HIPO
*/
class HIPO_Job_Create_Link extends HIPO_Job
{

	/**
	* The name of this HIPO job (should not be modified by code)
	* @var string
	*/
	var $_hipo_name = 'Create Link';

	/**
	* Array of steps to perform in this HIPO
	* @var Array
	*/
	var $_steps = Array(
					Array( //0
						'name'			=> 'Updating Lookups',
						'hipo_job'		=> Array(
												'init_details_function' => 'updateLookups',
												),
						'running_mode'	=> 'server',
						'auto_step'		=> true,
						'skip_step'		=> false,
						'percent_done'	=> 0,
						'complete'		=> false,
						'message'		=> '',
					),
					Array( //1
						'name'			=> 'Cascading Options',
						'function_call'		=> Array(
												'paint_function'   => 'paintInterface',
												'process_function' => 'processInterface',
												),
						'running_mode'	=> 'web',
						'auto_step'		=> false,
						'skip_step'		=> false,
						'percent_done'	=> 0,
						'complete'		=> false,
						'message'		=> '',
					),
					Array( //2
						'name'			=> 'Cascading Permissions',
						'hipo_job'		=> Array(
												'init_details_function' => 'CascadePermissions',
												),
						'running_mode'	=> 'server',
						'auto_step'		=> true,
						'skip_step'		=> false,
						'percent_done'	=> 0,
						'complete'		=> false,
						'message'		=> '',
					),
					Array( //3
						'name'			=> 'Cascading Metadata',
						'hipo_job'		=> Array(
												'init_details_function' => 'CascadeMetadata',
												),
						'running_mode'	=> 'server',
						'auto_step'		=> true,
						'skip_step'		=> false,
						'percent_done'	=> 0,
						'complete'		=> false,
						'message'		=> '',
					),
					Array( //4
						'name'			=> 'Cascading Workflow',
						'hipo_job'		=> Array(
												'init_details_function' => 'CascadeWorkflow',
												),
						'running_mode'	=> 'server',
						'auto_step'		=> true,
						'skip_step'		=> false,
						'percent_done'	=> 0,
						'complete'		=> false,
						'message'		=> '',
					),
				);


	/**
	* Constructor
	*
	* @param string	$code_name		a unique codename the HIPO
	*/
	function HIPO_Job_Edit_Create_Link($code_name='')
	{
		$this->HIPO_Job($code_name);

	}//end constructor


	/**
	* Returns a unique codename the HIPO can use to ensure it is not being run twice
	*
	* The Edit Permission HIPO appends the ID of the asset that the permissions are being set at
	* (the top most parent) so that any other permission jobs on this asset do not start while it
	* is running. Due to locking of the interface, nobody should be allowed to run this HIPO at
	* the same time on the same parent asset.
	*
	* @return string
	* @access private
	*/
	function _getCodeName()
	{
		return parent::_getCodeName().$this->_running_vars['assetid'];

	}//end _getCodeName()


	/**
	* Set up vars and database information that the HIPO will need to run
	*
	* The Edit Permissions HIPO also checks that there are permissions to edit and that
	* there is an assetid to start this action on (the top most parent). It also saves a
	* list of assets to perform this action on in the running vars.
	*
	* @param string	$source_code_name	the code name of another job that wants to use this job
	*
	* @return string	returns the code_name for the new job
	* @access public
	*/
	function initialise($source_code_name=null)
	{
		// to start editing permission, we need to have an asset id to start editing permission at
		if (!isset($this->_running_vars['assetid'])) {
			trigger_error('Cannot initialise Edit Permissions HIPO, no start asset ID specified', E_USER_WARNING);
			return '';
		}
		// so we have an assetid to start at, but make sure it is a valid assetid
		$asset = &$GLOBALS['SQ_SYSTEM']->am->getAsset($this->_running_vars['assetid']);
		if (is_null($asset)) {
			trigger_error('Cannot initialise Edit Permissions HIPO, invalid start asset ID specified ('.$this->_running_vars['assetid'].')', E_USER_WARNING);
			return '';
		}
		// so we have an parentid to start at, but make sure it is a valid assetid
		$asset = &$GLOBALS['SQ_SYSTEM']->am->getAsset($this->_running_vars['parentid']);
		if (is_null($asset)) {
			trigger_error('Cannot initialise Edit Permissions HIPO, invalid start parent ID specified ('.$this->_running_vars['parentid'].')', E_USER_WARNING);
			return '';
		}
		// we have a valid asset, so get all the assets below this one in the tree(s) so we know what assets
		// we will be affecting out list of changes on
		$this->_running_vars['todo_assetids'] = array_merge(Array($asset->id), $GLOBALS['SQ_SYSTEM']->am->getChildren($asset->id));
		$this->_running_vars['done_assetids'] = Array();

		unset($asset);
		return parent::initialise($source_code_name);

	}//end initialise()


	/**
	* Paints the interface for accepting user input on what to cascade (permissions/metadata/workflow)
	*
	* @param array()							&$step_data	a reference to the array of information about the current step
	* @param object Hipo_Backend_Outputter		&$o			the backend outputter class
	* @param string								$prefix		prefix for form vars
	*
	* @access Public
	* @return boolean
	*/
	function paintInterface(&$step_data, &$o, $prefix)
	{
		$hh = &$GLOBALS['SQ_SYSTEM']->getHipoHerder();
		if (is_null($hh)) return false;

		$o->addHiddenField('link_assetid',  $this->_running_vars['assetid']);
		$o->addHiddenField('link_parentid', $this->_running_vars['parentid']);

		$asset = &$GLOBALS['SQ_SYSTEM']->am->getAsset($this->_running_vars['assetid']);
		if (is_null($asset)) {
			trigger_error('Asset #'.$this->_running_vars['assetid'].' not found', E_USER_WARNING);
			return;
		}

		$parent = &$GLOBALS['SQ_SYSTEM']->am->getAsset($this->_running_vars['parentid']);
		if (is_null($parent)) {
			trigger_error('Asset #'.$this->_running_vars['parentid'].' not found', E_USER_WARNING);
			return;
		}

		$o->openSection('Note');
			$o->openField('&nbsp;');
				$o->addHiddenField('link_completed', '0');
				?>
				Press the <b><i>Cascade</i></b> button below if you want to cascade the permissions/workflow/metadata of "<?php echo $parent->name; ?>" down to "<?php echo $asset->name; ?>" and its children. To skip the cascading, press the <b><i>Skip</i></b> button below.<br/><br/>
				The permission of the new parent "<?php echo $parent->name; ?>" are shown below. These changes will attempt to be set on "<?php echo $asset->name; ?>" and its children if you choose to cascade.<br/><br/>
				<b>Note:</b> Any assets that become children of "<?php echo $asset->name; ?>" during the cascade process will not have changes applied to them - only the <i>current</i> children.
				<?php
				$ei = &$parent->getEI();
				
				$ei->includeStaticScreen('permissions');
				$pe = paintInlinePermissions($parent, $o, $ei);
				
				$ei->includeStaticScreen('workflow');
				$wf = paintInlineWorkflow($parent, $o, $ei);
				
				$ei->includeStaticScreen('metadataSchemas');
				$ms = paintInlineMetadataSchemas($parent, $o, $ei);
				
				$o->addHiddenField('form_submitted', 1);
			$o->closeField();
		$o->closeSection();
		
		$o->openSection('Cascade');
			$o->openField('', 'cascade');
				echo ($pe) ? 'Permissions'.check_box('cascade[]', 'cascade_pe').'<br />' : '';
				echo ($ms) ? 'Metadata Schemas'.check_box('cascade[]', 'cascade_ms').'<br />' : '';
				echo ($wf) ? 'Workflow'.check_box('cascade[]', 'cascade_wf').'<br />' : '';
			$o->closeField();
		$o->closeSection();
		
		return true;

	} // end paintInterface()


	/**
	* Process the user's selections to cascade whatever, and skip those steps that they don't want to run
	*
	* @param	&Array	$step_data		the step data for this job
	* @param	string	$prefix			prefix to be used with this job
	*
	* @access public
	* @return boolean
	*/
	function processInterface(&$step_data, $prefix)
	{
		// has the next button been clicked ?
		if (isset($_REQUEST['form_submitted'])) {
			if (isset($_REQUEST['cascade'])) {
				// permissions
				if (!in_array('cascade_pe', $_REQUEST['cascade'])) $this->skipStep(2);
				// metadata
				if (!in_array('cascade_ms', $_REQUEST['cascade'])) $this->skipStep(3);
				//workflow
				if (!in_array('cascade_wf', $_REQUEST['cascade'])) $this->skipStep(4);
			} else {
				$this->skipStep(2); // permissions
				$this->skipStep(3); // metadata
				$this->skipStep(4); // workflow
			}
			$step_data['percent_done'] = 100;
			$step_data['complete'] = true;
		}

		return true;
	
	} // end processInterface()
	

	/**
	* To be run as Step 1. This will run the Update Lookups before retrieving user input about cascading permissions etc.
	* Run as Step 1 as this is critical.
	*
	* @param string		&$job_type
	* @param Array()	&$running_vars
	* @param Array()	&$options
	*
	* @return void
	* @access public
	*/
	function updateLookups(&$job_type, &$running_vars, &$options)
	{
		$job_type = 'hipo_job_update_lookups';
		$options['auto_complete'] = true;
		$running_vars['assetids'] = Array($this->_running_vars['assetid']);

	}//end updateLookups()


	/**
	* This will run the Update Lookups before retrieving user input about cascading permissions etc.
	* Run as Step 1 as this is critical.
	*
	* @param string		&$job_type
	* @param Array()	&$running_vars
	* @param Array()	&$options
	*
	* @return void
	* @access public
	*/
	function cascadePermissions(&$job_type, &$running_vars, &$options)
	{
		$job_type = 'hipo_job_edit_permissions';
		$permission_vars = Array();

		foreach (Array(SQ_PERMISSION_READ, SQ_PERMISSION_WRITE, SQ_PERMISSION_ADMIN) as $perm) {
			$set_perms = $GLOBALS['SQ_SYSTEM']->am->getPermission($this->_running_vars['parentid'], $perm, null, false, false, true);
			foreach ($set_perms as $userid => $access) {
				$permission_vars[$perm][] = Array('access'          => $access,
												  'userid'          => $userid,
												  'previous_access' => null,
												  );
			}
		}

		$running_vars['permission_changes'] = $permission_vars;
		$running_vars['assetid'] = $this->_running_vars['assetid'];

		//$options['auto_complete'] = true;

	}//end cascadePermissions()


	/**
	* cascades to metadata (if not skipped)
	*
	* @param string		&$job_type
	* @param Array()	&$running_vars
	* @param Array()	&$options
	*
	* @return void
	* @access public
	*/
	function cascadeMetadata(&$job_type, &$running_vars, &$options)
	{
		$job_type = 'hipo_job_edit_metadata_schemas';
		$mm = &$GLOBALS['SQ_SYSTEM']->getMetadataManager();
		$schemas = $mm->getSchemas($this->_running_vars['parentid']);
		$schema_vars = Array();

		foreach ($schemas as $schemaid => $access) {
			$schema_vars[] = Array('access'          => $access,
								   'schemaid'        => $schemaid,
								   'previous_access' => null,
								   );
		}
		$running_vars['schema_changes'] = $schema_vars;
		$running_vars['assetid'] = $this->_running_vars['assetid'];

		//$options['auto_complete'] = true;

	}//end cascadeMetadata()


	/**
	* cascades workflow (if not skipped)
	*
	* @param string		&$job_type
	* @param Array()	&$running_vars
	* @param Array()	&$options
	*
	* @return void
	* @access public
	*/
	function cascadeWorkflow(&$job_type, &$running_vars, &$options)
	{
		$job_type = 'hipo_job_edit_workflow_schemas';
		$wfm = &$GLOBALS['SQ_SYSTEM']->getWorkflowManager();
		$schemas = $wfm->getSchemas($this->_running_vars['parentid']);
		$schema_vars = Array();

		foreach ($schemas as $schemaid => $access) {
			$schema_vars[] = Array('access'          => $access,
								   'schemaid'        => $schemaid,
								   'previous_access' => null,
						      );
		}
		$running_vars['schema_changes'] = $schema_vars;
		$running_vars['assetid'] = $this->_running_vars['assetid'];

		//$options['auto_complete'] = true;

	}//end cascadeWorkflow()

}//end class

?>