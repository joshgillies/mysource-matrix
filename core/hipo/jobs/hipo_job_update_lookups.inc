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
* $Id: hipo_job_update_lookups.inc,v 1.60 2012/10/23 23:31:18 cupreti Exp $
*
*/

require_once SQ_SYSTEM_ROOT.'/core/hipo/hipo_job.inc';

/**
* Highly Intensive Processing Object (HIPO) to update lookups
*
* <pre>
* Initial Running Vars :
*    assetids	-	an array of id's of assets that we will be starting the update from
* </pre>
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.60 $
* @package MySource_Matrix
* @subpackage HIPO
*/
class HIPO_Job_Update_Lookups extends HIPO_Job
{


	/**
	* Constructor
	*
	* @param string	$code_name		a unique codename the HIPO
	*/
	function HIPO_Job_Update_Lookups($code_name='')
	{
		$this->HIPO_Job($code_name);

	}//end constructor


	/**
	* Paints backend of this job
	*
	* @param object	&$o		reference to the backend outputter
	* @param string	$class	calling class name
	*
	* @return void
	* @access private
	*/
	public static function paintConfig(&$o, $class, $write_access)
	{
		// metadata regeneration threshhold HIPO config entry
		$o->openField(translate('lookups_updating_threshold'));

		if ($write_access) {
			text_box($class.'[SQ_HIPO_LOOKUPS_THRESHOLD]', SQ_HIPO_LOOKUPS_THRESHOLD, 5);
		} else {
			echo SQ_HIPO_LOOKUPS_THRESHOLD;
		}
		echo ' '.translate('assets');

		$o->closeField();

	}//end paintConfig()


	/**
	* Sets some info about thresholds that this hipo requires.
	*
	* @return array
	* @access public
	*/
	public static function getConfigVars()
	{
		return Array(
				'SQ_HIPO_LOOKUPS_THRESHOLD'	=> Array('editable'	=> 1, 'default'	=> 1),
			   );

	}//end getConfigVars()


	/**
	* Returns a unique codename the HIPO can use to ensure it is not being run twice
	*
	* The Update Lookups HIPO appends the ID of the asset that the lookups are being updated at
	* (the top most parent) so that any other update lookup jobs on this asset do not start while it
	* is running. Due to locking of the interface, nobody should be allowed to run this HIPO at
	* the same time on the same parent asset.
	*
	* @return string
	* @access public
	*/
	function getCodeName()
	{
		return parent::getCodeName().'-'.md5(implode('-', $this->_running_vars['assetids']));

	}//end getCodeName()


	/**
	* Returns the (localised) HIPO name
	*
	* @return string
	* @access public
	*/
	function getHipoName()
	{
		return translate('hipo_name_update_lookups');

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
					'name'			=> translate('hipo_updating_lookups'),
					'function_call'	=> Array(
										'process_function'	=> 'processLookups',
									   ),
					'running_mode'	=> 'server',
					'auto_step'		=> TRUE,
					'allow_cancel'	=> TRUE,
					'percent_done'	=> 0,
					'complete'		=> FALSE,
					'message'		=> '',
				),
			   );

	}//end getInitialStepData()


	/**
	* Returns the current Threshold.
	*
	* Returns a value that represents the total percentage that this hipo would run in freestyle mode in its current state.
	*
	* @return float
	* @access public
	*/
	function getThresholdPercentageRequired()
	{
		if (SQ_HIPO_LOOKUPS_THRESHOLD == 0) return 0;
		if (!isset($this->_running_vars['todo_assetids'])) {
			return 0;
		}
		return ((count($this->_running_vars['todo_assetids']) / SQ_HIPO_LOOKUPS_THRESHOLD) * 100);

	}//end getThresholdPercentageRequired()


	/**
	* Performs the duties of this hipo, without showing any output on the frontend.
	*
	* @return boolean
	* @access public
	*/
	function freestyle()
	{
		while (!empty($this->_running_vars['todo_assetids'])) {
			if (!$this->processLookups($this->_steps[0], get_class($this))) {
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
		// to start updating lookups, we need to have an asset id to start updating lookups at
		if (empty($this->_running_vars['assetids'])) {
			trigger_localised_error('HIPO0001', E_USER_WARNING);
			return FALSE;
		}
		$db = MatrixDAL::getDb();

		$todo_normal = Array();
		$todo_shadows = Array();

		foreach ($this->_running_vars['assetids'] as $assetid) {
			// check if we are updating lookups for a shadow asset, or a bridge
			$id_parts = explode(':', $assetid);
			if (isset($id_parts[1])) {
				$todo_shadows = array_merge($todo_shadows, array_keys($GLOBALS['SQ_SYSTEM']->am->getChildren($assetid)));
			} else {
				$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
				if ($asset instanceof Bridge) {
					if (!method_exists($asset, 'getChildren')) {
						trigger_localised_error('SYS0204', E_USER_WARNING, $asset->name);
					} else {
						$todo_shadows = array_merge($todo_shadows, array_keys($asset->getChildren($assetid)));
					}
				}

				$where = 'l.minorid = :assetid';
				$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 't');
				$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'l');
				$sql = 'SELECT t.treeid
						FROM '.SQ_TABLE_RUNNING_PREFIX.'ast_lnk_tree t INNER JOIN '.SQ_TABLE_RUNNING_PREFIX.'ast_lnk l ON t.linkid = l.linkid
						'.$where;
				$sql = db_extras_modify_limit_clause($sql, MatrixDAL::getDbType(), 1);

				try {
					$query = MatrixDAL::preparePdoQuery($sql);
					MatrixDAL::bindValueToPdo($query, 'assetid', $assetid);
					$treeid = MatrixDAL::executePdoOne($query);
				} catch (Exception $e) {
					throw new Exception('Unable to get treeid for minorid: '.$assetid.' due to database error: '.$e->getMessage());
				}

				$sql = 'SELECT l.minorid, MAX(LENGTH(t.treeid)) as length
						FROM '.SQ_TABLE_RUNNING_PREFIX.'ast_lnk_tree t
								 INNER JOIN '.SQ_TABLE_RUNNING_PREFIX.'ast_lnk l ON t.linkid = l.linkid
						AND (('.db_extras_bitand(MatrixDAL::getDbType(), 'l.link_type', SQ_SC_LINK_BACKEND_NAV).' > 0) OR 
							 (('.db_extras_bitand(MatrixDAL::getDbType(), 'l.link_type', SQ_SC_LINK_SIGNIFICANT).' > 0) AND ';
				$sql .= (MatrixDAL::getDbType() === 'oci') ? '(l.value IS NOT NULL)))' : '(l.value <> \'\'))) ';
				$where = 't.treeid LIKE :treeid
						  GROUP BY l.minorid ORDER BY length';

				$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 't');
				$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'l');

				try {
					$query = MatrixDAL::preparePdoQuery($sql.$where);
					MatrixDAL::bindValueToPdo($query, 'treeid', $treeid.'%');
					$new_assets = MatrixDAL::executePdoAssoc($query);
				} catch (Exception $e) {
					throw new Exception('Unable to get minorids for treeid: '.$treeid[0]['treeid'].' due to database error: '.$e->getMessage());
				}

				foreach ($new_assets as $new_asset) {
				    $todo_normal []= $new_asset;
				}
			}//end else

		}//end foreach

		// Make sure lower assets are done after higher ones
		$todo_assetids = Array();
		if (!empty($todo_normal)) {
			usort($todo_normal, create_function('$a, $b', 'return $a[\'length\'] > $b[\'length\'];'));
			foreach($todo_normal as $asset_info) {
				$todo_assetids[] = $asset_info['minorid'];
			}
			unset($todo_normal);
			unset($new_assets);
		}

		// Whether to add auto remap
		$rm = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('remap_manager');
		$auto_add_remaps = $rm->attr('remap_upon_webpath_change');

		$this->_running_vars['todo_assetids'] = array_unique(array_merge($todo_assetids, $todo_shadows));
		if (!isset($this->_running_vars['auto_add_remaps'])) $this->_running_vars['auto_add_remaps'] = $auto_add_remaps;
		$this->_running_vars['done_assetids'] = Array();
		return parent::prepare();

	}//end prepare()


	/**
	* Update lookups for a single asset in the to-do list
	*
	* @param array	&$step_data	a reference to the array of information about the current step
	* @param string	$prefix		prefix for form vars
	*
	* @return boolean
	* @access public
	*/
	function processLookups(&$step_data, $prefix)
	{
		if (!empty($this->_running_vars['todo_assetids'])) {
			$assetid = array_shift($this->_running_vars['todo_assetids']);
			$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);

			if (!is_null($asset)) {
				if (!$asset->updateLookups($this->_running_vars['auto_add_remaps'])) return FALSE;
				$step_data['message'] = translate('hipo_updating_lookups_for', htmlentities($asset->name, ENT_COMPAT, SQ_CONF_DEFAULT_CHARACTER_SET));
				$GLOBALS['SQ_SYSTEM']->am->forgetAsset($asset);
				unset($asset);
			} else {
				$step_data['message'] = translate('hipo_skipping_asset', $assetid);
				$this->_addError(translate('hipo_cannot_update_lookups', $assetid));
			}

			// add this assetid to the done array so we dont do it again
			$this->_running_vars['done_assetids'][] = $assetid;
		}

		if (empty($this->_running_vars['todo_assetids'])) {
			$step_data['percent_done'] = 100;
			$step_data['complete']     = TRUE;
		} else {
			$total = count($this->_running_vars['todo_assetids']) + count($this->_running_vars['done_assetids']);
			$step_data['percent_done'] =(count($this->_running_vars['done_assetids']) / $total) * 100;
			$step_data['complete']     = FALSE;
		}

		return TRUE;

	}//end processLookups()


}//end class

?>
