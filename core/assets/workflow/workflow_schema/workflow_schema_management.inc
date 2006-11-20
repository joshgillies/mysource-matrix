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
* $Id: workflow_schema_management.inc,v 1.8 2006/11/20 05:20:52 emcdonald Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Workflow_Schema_Management
*
* @see Asset_Management
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.8 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Workflow_Schema_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function Workflow_Schema_Management(&$pm)
	{
		$this->Asset_Management($pm);

		$this->vars = Array(
						'name'	=> Array(
									'added'		=> '0.1',
									'type'		=> 'text',
									'default'	=> '',
								   ),
					  );

	}//end constructor


	/**
	* Upgrade
	*
	* @param string	$current_version	the version of the asset that is currenly installed
	*
	* @return boolean
	* @access public
	*/
	function _upgrade($current_version)
	{
		// first do things as normal
		$res = parent::_upgrade($current_version);

		if (version_compare($current_version, '0.2', '<')) {
			echo('UPGRADING FORM QUESTION FROM v < 0.2');

			$db =& $GLOBALS['SQ_SYSTEM']->db;
			$sql = 'SELECT assetid, schemaid, wflow
					FROM '.SQ_TABLE_RUNNING_PREFIX.'ast_wflow
					WHERE wflow IS NOT NULL
					';
			$rows = $db->getAll($sql);
			assert_valid_db_result($rows);

			$wfm =& $GLOBALS['SQ_SYSTEM']->getWorkflowManager();
			$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
			$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
			$db =& $GLOBALS['SQ_SYSTEM']->db;

			$sql = 'UPDATE
						sq_ast_wflow
					SET
						wflow	= ?
					WHERE
							assetid		= ?
						AND	schemaid	= ?';

			foreach ($rows as $row) {

				$workflow = unserialize($row['wflow']);
				$schemaid = $row['schemaid'];
				$assetid = $row['assetid'];

				if (!is_array($workflow['current_step'])) {
					$workflow['current_step'] = Array($workflow['current_step']);
				}

				foreach ($workflow['steps'] as $stepid => $step) {
					$step_data =& $workflow['steps'][$stepid];
					$step_data['escalation_steps']	= Array();
					$step_data['escalate']			= FALSE;
					$step_data['expired']			= FALSE;

				}

				$prepared = $db->prepare($sql);
				assert_valid_db_result($prepared);
				$new_workflow = serialize($workflow);

				$values = Array(
							$new_workflow,
							$assetid,
							$schemaid,
						  );


				$result = $db->execute($prepared, $values);

				assert_valid_db_result($result);

			}//end foreach
			$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

			unset($rows);

		}//end if
		return TRUE;

	}//end _upgrade()


}//end class

?>
