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
* $Id: search_manager.inc,v 1.1 2004/05/13 04:05:26 lwright Exp $
* $Name: not supported by cvs2svn $
*/


/**
* Search_Manager
*
* Manages searching and relevance weights for attributes of asset types.
*
*
* @author  Dominic Wong <dwong@squiz.net>
* @version $Revision: 1.1 $ - 0.1
* @package MySource_Matrix_Packages
* @subpackage __core__
*/

class Search_Manager extends Asset
{


	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function Search_Manager($assetid=0)
	{
		$this->Asset($assetid);

	}// end constructor


	/**
	* Create this asset
	*
	* @param array()	&$link	information used to create the initial link
	*
	* @see Asset::create()
	* @return mixed int or false
	* @access public
	*/
	function create(&$link)
	{
		require_once SQ_CORE_PACKAGE_PATH.'/system/system_asset_fns.inc';
		if (!system_asset_fns_create_pre_check($this)) return false;
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		if ($linkid = parent::create($link)) {
			if (!system_asset_fns_create_cleanup($this))  {
				$linkid = false;
			}
		}

		if ($linkid) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		} else {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
		}

		return $linkid;

	}//end create()


	/**
	* Returns name of the asset
	*
	* @param bool	$short_name	whether or not we are after the shortname or the full name
	*
	* @see Asset::_getName()
	* @access public
	*/
	function _getName($short_name=false)
	{
		return $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($this->type(), 'name');

	}//end _getName()


	/**
	* Can this asset have its last significant link removed (putting it in the trash)?
	*
	* @return boolean
	* @access public
	*/
	function canDelete()
	{
		return false;

	}//end canDelete()


	/**
	* returns FALSE always because we don't want to be able to clone a search manager
	*
	* @return boolean
	* @access public
	*/
	function canClone()
	{
		return false;
	
	}//end canClone()


	/**
	* Event fired when an asset messages that it requires its stuff to be reindexed
	*
	* @param &object Asset	&$broadcaster	the asset that triggered the event
	* @param Array			$vars			the vars that get submitted by the broadcaster
	*										Component is set as $vars['component']
	*
	* @return boolean
	* @access private
	*/
	function onRequestReindex(&$broadcaster, $vars=Array())
	{
		//print "Require Reindex is being run.";

		// if this is reindexed then there is an infinite recursion!!!
		if ($broadcaster->id == $this->id) return false;

		$component = isset($vars['component']) ? $vars['component'] : null;
		$am =& $GLOBALS['SQ_SYSTEM']->am;
		$am->acquireLock($this->id, 'attributes');

		// get dependent parents - but put broadcaster first
		$changed_assetids = Array($broadcaster->id) + $am->getDependantParents($broadcaster->id);

		// for each of parents+current asset, refresh key data for specified component
		foreach ($changed_assetids as $assetid) {
			$asset = &$am->getAsset($assetid);
			$this->flushKeyData($assetid, $component);
			$new_key_data = $asset->getKeyData();
			$this->addKeyData($assetid, $new_key_data);
			$am->forgetAsset($assetid);
		}

		$am->releaseLock($this->id, 'attributes');

		return true;

	}//end onRequireReindex()
	
	
	/**
	* Flush the keydata for an asset
	*
	* @param int		$assetid	ID of the asset
	* @param string	$component	the component to which the keydata pertains (e.g. metadata or attribute)
	*
	* @return void
	* @access public
	*/
	function flushKeyData($assetid, $component=null)
	{
		$db =& $GLOBALS['SQ_SYSTEM']->db;
		$sql = 'DELETE FROM '.SQ_TABLE_PREFIX.'asset_indexing
				WHERE assetid = '.$db->quote($assetid);
		if (!is_null($component)) $sql .= ' AND component = '.$db->quote($component);

		$result = $db->query($sql);
		if (DB::isError($result)) {
			trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_WARNING);
		}

	}//end flushKeyData()
	
	
	/**
	* Adds keydata for an asset
	*
	* @param int		$assetid	ID of the asset
	* @param array		&$keydata	an array of the form :
	*<pre>
	*	Array (
	*		0 => Array (
	*				'value'     => ... ,
	*				'type'      => ... ,
	*				'component' => ... (optional, defaults to '')
	*				), ...
	*		)
	* </pre>
	* @return boolean
	* @access public
	*/
	function addKeyData($assetid, &$keydata)
	{
		if (!SQ_CONF_INDEXING_ENABLED) return true;
		if (!is_array($keydata)) return false;

		$db = &$GLOBALS['SQ_SYSTEM']->db;

		$param_data = Array();
		for (reset($keydata); null !== ($i = key($keydata)); next($keydata)) {
			$datum =& $keydata[$i];
			if ($datum['type'] == 'date') {
				// convert to ISO 8601 date format Y-m-d H:i:s if not already
				if (!(ereg('[0-9]{4}-[0-9]{2}-[0-9]{2} [0-9]{2}:[0-9]{2}:[0-9]{2}', $datum['type']))) {
					$datum['value'] = date('Y-m-d H:i:s', strtotime($datum['value']));
				}
			} else if ($datum['type'] == 'text') {
				$datum['value'] = strtolower($datum['value']);
			}
			array_push($param_data, Array($datum['value'], $datum['type'], $datum['component'], $datum['score']));
		}

		if (empty($param_data)) return false;
		$st = $db->prepare(
			'INSERT INTO '.SQ_TABLE_PREFIX.'asset_indexing(value, type, assetid, component, score)
			 VALUES (?, ?, '.$db->quote($assetid).', ?, ?)');

		$result = $db->executeMultiple($st, $param_data);
		if (DB::isError($result)) {
			trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_WARNING);
			return false;
		}

		return true;

	}//end addKeyData()
	
	
	/**
	* Sets the default component weighting for a particular asset type
	*
	* This does NOT create any new rows in defaults
	*
	* @param string		$type_code	the type of asset
	* @param string		$component	the component being wieghted
	* @param int		$weighting	the new weighting of the component
	*
	* @return void
	* @access public
	*/
	function setComponentDefaultWeighting($type_code, $component, $weighting)
	{
		$db =& $GLOBALS['SQ_SYSTEM']->db;

		if (strlen($component) == 0)
			continue;

		$sql = 'UPDATE '. SQ_TABLE_PREFIX .'asset_indexing_weightings_default
				SET weighting = ' . $db->quote($weighting) .'
				WHERE type_code = '. $db->quote($type_code) . ' AND component = '. $db->quote($component) ;

		$result = $db->query($sql);
		if (DB::isError($result)) {
			trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_WARNING);
		}

	}//end setComponentDefaultWeighting()


	/**
	* Sets the component weighting for a particular asset
	*
	* @param int				$assetid		the asset ID
	* @param int				$component		the component
	* @param mixed (null|int)	$weighting		the weighting - if null, reverts back to the default for this asset's type
	*
	* @return void
	* @access public
	*/
	function setComponentWeighting($assetid, $component, $weighting=null)
	{
		$db =& $GLOBALS['SQ_SYSTEM']->db;

		$sql = 'DELETE FROM '.SQ_TABLE_PREFIX.'asset_indexing_weightings
				WHERE assetid = '.$db->quote($assetid).' AND component = '.$db->quote($component);
		$result = $db->query($sql);
		if (DB::isError($result)) {
			trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_WARNING);
		}

		if (!is_null($weighting)) {
			$sql = 'INSERT INTO '.SQ_TABLE_PREFIX.'asset_indexing_weightings(assetid, component, weighting)
					VALUES ('.
						$db->quote($assetid)   .', '.
						$db->quote($component) .', '.
						$db->quote($weighting) .
					')';
			$result = $db->query($sql);
			if (DB::isError($result)) {
				trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_WARNING);
			}
		}

	}//end setComponentWeighting()
	
	
	/**
	* Retrieves weightings for this asset and its components - loads on demand.
	*
	* @return Array
	* @access public
	*/
	function &getAssetWeightings($assetid)
	{
		$db =& $GLOBALS['SQ_SYSTEM']->db;

		// load the weightings
		$sql = 'SELECT wd.component, wd.description, (w.weighting IS NULL) as def, COALESCE(w.weighting, wd.weighting) as weighting
				FROM
					'. SQ_TABLE_PREFIX .'asset_indexing_weightings_default wd
						LEFT OUTER JOIN '. SQ_TABLE_PREFIX. 'asset a
							ON wd.type_code = a.type_code
						LEFT OUTER JOIN '.SQ_TABLE_PREFIX .'asset_indexing_weightings w
							ON w.component = wd.component AND
							a.assetid = w.assetid
				WHERE
					a.assetid = ' . $db->quote($assetid) ;

		$results = $db->getAll($sql);
		if (DB::isError($results)) {
			trigger_error($results->getMessage().'<br/>'.$results->getUserInfo(), E_USER_ERROR);
			return false;
		}

		$weightings = Array();
		foreach ($results as $result) {
			$weightings[$result['component']] =
				Array(
					'default'     => ($result['def'] == 't') ? 1 : 0,
					'weighting'   => $result['weighting'],
					'description' => $result['description'],
				);
		}

		return $weightings;

	}// end getAssetWeightings()

	
	
	/**
	* Processes a search request
	*
	* Formulates search query and returns the results
	*
	* @return Array()
	* @access private
	*/
	function processSearch($search_info)
	{

		$db = &$GLOBALS['SQ_SYSTEM']->db;
		$total_asset_scores = null;


		  ///////////////////////////////////
		 //  CONSTRUCT BASE SEARCH QUERY  //
		///////////////////////////////////
		$concat = ($db->phptype == 'mysql') ? 'CONCAT(alt1.treeid, '.$db->quote('%').')' : '(alt1.treeid || '.$db->quote('%').')';

		$start_sql = 'SELECT DISTINCT(ai.assetid), SUM(ai.score)
						FROM
							'.SQ_TABLE_PREFIX.'asset_indexing ai,
							'.SQ_TABLE_PREFIX.'asset_link_tree alt1,
							'.SQ_TABLE_PREFIX.'asset_link_tree alt2,
							'.SQ_TABLE_PREFIX.'asset_link al1,
							'.SQ_TABLE_PREFIX.'asset_link al2,
							'.SQ_TABLE_PREFIX.'asset_permission ap,
							'.SQ_TABLE_PREFIX.'asset a
						WHERE
							al1.majorid = '.$db->quote($search_info['root_assetid']).'
							AND al1.linkid = alt1.linkid
							AND alt2.treeid LIKE '.$concat.'
							AND alt2.linkid = al2.linkid
							AND ai.assetid  = al2.minorid
							AND a.assetid   = ai.assetid
							AND ap.assetid  = ai.assetid
							AND a.status  >= '.$db->quote(SQ_STATUS_LIVE);

		if (!empty($search_info['search_types'])) {
			for(reset($search_info['search_types']); NULL !== ($i = key($search_info['search_types'])); next($search_info['search_types'])) {
				$search_info['search_types'][$i] = $db->quote($search_info['search_types'][$i]);
			}
			$type_code_cond = 'IN ('.implode(', ', $search_info['search_types']).')';

			require_once SQ_FUDGE_PATH.'/db_extras/db_extras.inc';
			$start_sql .= ' AND a.type_code IN (~SQ0~)';
			$subs = Array( 'SELECT type_code
							FROM '.SQ_TABLE_PREFIX.'asset_type_inherited
							WHERE inherited_type_code '.$type_code_cond );
			$start_sql = db_extras_subquery($db, $start_sql, $subs);
			if (DB::isError($start_sql)) {
				trigger_error($start_sql->getMessage().'<br/>'.$start_sql->getUserInfo(), E_USER_ERROR);
			}
		}

		$userid_cond = '';
		if ($GLOBALS['SQ_SYSTEM']->currentUserId()) {
			if (!$GLOBALS['SQ_SYSTEM']->userRoot() && !$GLOBALS['SQ_SYSTEM']->userSystemAdmin()) {
				$userids = $GLOBALS['SQ_SYSTEM']->am->getParents($GLOBALS['SQ_SYSTEM']->user->id, 'user_group', false);
				array_push($userids, 0, (int) $GLOBALS['SQ_SYSTEM']->user->id);
				$userid_cond = ' AND ap.userid  IN ('.implode(',', $userids).')';
			}
		} else {
			// public users
			$userid_cond = ' AND ap.userid = 0';
		}

		if (!empty($userid_cond)) {
			$start_sql .= $userid_cond;
			$start_sql .= ' AND ap.permission >= '.$db->quote(SQ_PERMISSION_READ).'
							AND ap.access = 1';
		}


		  ///////////////////////////////
		 //  SEARCH ASSET ATTRIBUTES  //
		///////////////////////////////
		if (isset($search_info['asset_attrib'])) {

			foreach($search_info['asset_attrib'] as $attr_name => $attr_info) {

				if ($attr_info['params']['searchtype'] == '') continue;
				$attr_parts = explode('__', $attr_info['params']['type_attr']); // exploding the name
				$field_asset_scores = null;

				foreach ($attr_info['words'] as $word) {

					$sql = $start_sql.'
						AND ai.component = '.$db->quote('attr:'. $attr_info['params']['searchtype'].':'.$attr_parts[0]).'
						AND (ai.value LIKE '.$db->quote('%'.$word.'%').')
						GROUP BY ai.assetid';

					$result = $db->getAssoc($sql);
					if (DB::isError($result)) trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_ERROR);

					if (is_null($field_asset_scores)) $field_asset_scores = $result;
					else $field_asset_scores = $this->combineAssetScores($field_asset_scores, $result, 'AND');
				}

				if (is_null($total_asset_scores)) $total_asset_scores = $field_asset_scores;
				else $total_asset_scores = $this->combineAssetScores($total_asset_scores, $field_asset_scores, $search_info['logic']);
			}
		}


		  /////////////////////////////////
		 //  SEARCH INCLUDE ALL FIELDS  //
		/////////////////////////////////
		if (isset($search_info['include_all'])) {

			foreach($search_info['include_all'] as $attr_name => $attr_info) {

				$field_asset_scores = null;
				foreach ($attr_info['words'] as $word) {
					$sql = $start_sql.'
						AND (ai.value LIKE '.$db->quote('%'.$word.'%').')
						GROUP BY ai.assetid';

					$result = $db->getAssoc($sql);
					if (DB::isError($result)) trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_ERROR);

					if (is_null($field_asset_scores)) $field_asset_scores = $result;
					else $field_asset_scores = $this->combineAssetScores($field_asset_scores, $result, 'AND');
				}

				if (is_null($total_asset_scores)) $total_asset_scores = $field_asset_scores;
				else $total_asset_scores = $this->combineAssetScores($total_asset_scores, $field_asset_scores, $search_info['logic']);
			}
		}


		  ///////////////////////
		 //  SEARCH METADATA  //
		///////////////////////
		if (isset($search_info['metadata'])) {

			foreach($search_info['metadata'] as $attr_name => $attr_info) {

				if ($attr_info['params']['metadata_field']['assetid'] <= 0) continue;

				$field_asset_scores = null;
				foreach ($attr_info['words'] as $word) {

					if (!is_array($word)) {

						$like = $db->quote('%'.$word.'%');

					} else {

						// we are processing a date field
						$value = $word;

						$myday = $value['d'];
						if (strlen($myday) == 1) $myday = '0'.$myday;
						if (empty($myday) || $myday == '--') $myday = '__';

						$mymonth = $value['m'];
						if (strlen($mymonth) == 1) $mymonth = '0'.$mymonth;
						if (empty($mymonth) || $mymonth == '--') $mymonth = '__';

						$myyear = $value['y'];
						if (empty($myyear) || $myyear == '--') $myyear = '__';

						$myhour = $value['h'];
						if (strlen($myhour) == 1) $myhour = '0'.$myhour;
						if (empty($myhour) || $myhour == '--') $myhour = '__';

						$myminute = $value['i'];
						if (strlen($myminute) == 1) $myminute = '0'.$myminute;
						if (empty($myminute) || $myminute == '--') $myminute = '__';

						$mysecond = $value['s'];
						if (strlen($mysecond) == 1) $mysecond = '0'.$mysecond;
						if (empty($mysecond) || $mysecond == '--') $mysecond = '__';

						$like = $myday.':'.$mymonth.':'.$myyear.':'.$myhour.':'.$myminute.':'.$mysecond;
						if ($like == '__:__:__:__:__:__') continue;
						$like = $db->quote($like);
					}

					$sql = $start_sql.'
							AND ai.type = '.$db->quote('text').'
							AND ai.component = '.$db->quote('metadata:'.(int)$attr_info['params']['metadata_field']['assetid']).'
							AND ai.value LIKE '.$like.'
							GROUP BY ai.assetid';

					$result = $db->getAssoc($sql);
					if (DB::isError($result)) trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_ERROR);

					if (is_null($field_asset_scores)) $field_asset_scores = $result;
					else $field_asset_scores = $this->combineAssetScores($field_asset_scores, $result, 'AND');
				
				}

				if (is_null($total_asset_scores)) $total_asset_scores = $field_asset_scores;
				else $total_asset_scores = $this->combineAssetScores($total_asset_scores, $field_asset_scores, $search_info['logic']);
			}
		}

		  /////////////////////////////
		 //  SEARCH EXCLUDE FIELDS  //
		/////////////////////////////
		if (isset($search_info['exclude'])) {

			foreach($search_info['exclude'] as $attr_name => $attr_info) {

				$sql = $start_sql.'
					AND (ai.value LIKE \'%'.implode ("%' OR ai.value LIKE '%", $attr_info['words']).'%\')
					GROUP BY ai.assetid';

				$result = $db->getAssoc($sql);
				if (DB::isError($result)) trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_ERROR);

				foreach (array_keys($results) as $exclude_assetid) {
					if (isset($total_asset_scores[$exclude_assetid])) unset($total_asset_scores[$exclude_assetid]);
				}
			}
		}
		
		if (is_null($total_asset_scores)) $total_asset_scores = Array();
		else arsort($total_asset_scores);
		return $total_asset_scores;

	}//end processSearch()


	/**
	* Combines asset scores according to the passed logic
	*
	* @param array	$asset_scores1		a mapping from assetids to scores
	* @param array	$asset_scores2		a mapping from assetids to scores
	* @param string	$logic				either 'AND' or 'OR'
	*
	* @return Array a mapping from assetids to combined scores
	* @access private
	*/
	function combineAssetScores($asset_scores1, $asset_scores2, $logic='AND')
	{
		if (!is_array($asset_scores1) && !is_array($asset_scores2)) return Array();
		else if (!is_array($asset_scores2)) return $asset_scores1;
		else if (!is_array($asset_scores1)) return $asset_scores2;

		// find the intersection/union (depending on logic) of the assetids in field_asset_scores and results
		$assets1 = array_keys($asset_scores1);
		$assets2 = array_keys($asset_scores2);

		$out_asset_scores = $asset_scores1;

		if ($logic == 'AND') {

			$common_assets = array_intersect($assets1, $assets2);

			// get the assets that are to be removed in field_asset_scores
			$removed_assets = array_diff($assets1, $assets2);
			foreach ($removed_assets as $assetid) unset ($out_asset_scores[$assetid]);

			foreach ($common_assets as $assetid) {
				if (isset($out_asset_scores[$assetid])) $out_asset_scores[$assetid] += $asset_scores2[$assetid];
				else $out_asset_scores[$assetid] = $asset_scores2[$assetid];
			}

		} else if ($logic == 'OR') {

			foreach ($asset_scores2 as $assetid => $score) {
				if (isset($out_asset_scores[$assetid])) $out_asset_scores[$assetid] += $asset_scores2[$assetid];
				else $out_asset_scores[$assetid] = $asset_scores2[$assetid];
			}

		}

		return $out_asset_scores;

	}//end combineAssetScores()


}//end class

?>
