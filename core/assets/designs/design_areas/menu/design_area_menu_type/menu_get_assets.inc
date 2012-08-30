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
* $Id: menu_get_assets.inc,v 1.19 2012/08/30 01:09:06 ewang Exp $
*
*/


/**
* Returns an array of assetids corresponding to valid menu entries
*
* @param string	$id_name	the name of the menu design area
* @param string	$majorids	an array of major ids for each level of the menu
*
* @return array
* @access public
*/
function menu_get_assets($id_name, $majorids, $ignore_permissions = FALSE)
{

	$db = MatrixDAL::getDb();

	if (!isset($GLOBALS['MENU_DATA'][$id_name])) {
		$GLOBALS['MENU_DATA'][$id_name] = Array();
	}
	$storage =& $GLOBALS['MENU_DATA'][$id_name]; // need ref
	$assetids = Array();
	$majorids_str = '';
	foreach ($majorids as $id) {
		if (empty($storage[$id])) {
			$majorids_str .= '\''.(string)$id.'\',';
		} else {
			foreach ($storage[$id] as $row) {
				$assetids[] = (int) $row['assetid'];
			}
		}
	}

	if (empty($majorids_str)) return $assetids;

	// Watch out for this static var, once it has been set, it won't go into the if is_null. However the bind vars will need to be there because this static var contain the query that uses bind vars.
	static $USERIDS_COND = NULL;
	static $USERIDS_BIND_VARS = NULL;
	$bind_vars = Array();

	$public_userid = $GLOBALS['SQ_SYSTEM']->am->getSystemAssetid('public_user');
	if (is_null($USERIDS_COND)) {
		// if they are logged in add their parents to the list
		if ($GLOBALS['SQ_SYSTEM']->userRoot() || $GLOBALS['SQ_SYSTEM']->userSystemAdmin() || $ignore_permissions) {
			$USERIDS_COND = '';
		} else {
			$user = $GLOBALS['SQ_SYSTEM']->am->getAsset($GLOBALS['SQ_SYSTEM']->currentUserId());
			$userids = $user->getUserGroups();
			$GLOBALS['SQ_SYSTEM']->am->forgetAsset($user);
			$userids[] = $public_userid;
			$userids[] = $GLOBALS['SQ_SYSTEM']->user->id;
			$userids_bind_vars = Array();
			$userids_vars_p = Array();
			$userids_vars_r = Array();
			for (reset($userids); NULL !== ($i = key($userids)); next($userids)) {
				$userids_vars_p[] = ':mga_userids_p_'.$i;
				$userids_bind_vars['mga_userids_p_'.$i] = (string) $userids[$i];
				$userids_vars_r[] = ':mga_userids_r_'.$i;
				$userids_bind_vars['mga_userids_r_'.$i] = (string) $userids[$i];
			}//end for
			$USERIDS_BIND_VARS = serialize($userids_bind_vars);
			$userids_p_str = implode(', ', $userids_vars_p);
			$userids_r_str = implode(', ', $userids_vars_r);
			$USERIDS_COND = 'AND (p.userid  IN ('.$userids_p_str.') ';
			$USERIDS_COND .= 'OR r.userid  IN ('.$userids_r_str.')) ';
			$USERIDS_COND .= ' AND (
				(p.permission = :p_permission AND (
						p.userid <> :p_userid
						OR r.userid <> :r_userid
						OR (p.userid = :p_userid_1 AND p.granted = \'1\')
						OR (r.userid = :r_userid_1 AND p.granted = \'1\')
					)
				)
				OR	(
						p.permission > :p_permission_1 AND p.granted = \'1\'
					)
			)';
			$USERIDS_COND .= '
							  GROUP BY a.assetid, l.majorid, a.type_code, a.status, a.name, a.short_name, pt.path, l.sort_order, p.assetid
							  HAVING MIN(p.granted) <> \'0\'';

		}//end else

	}//end if

	// This check to avoid the case when the viewing entity is root user or sys admins, trying to bind vars to an empty query will result in Fatal error from Oracle. Need to be explicitly not empty string.
	if ($USERIDS_COND !== '') {
		$bind_vars['p_permission']   = SQ_PERMISSION_READ;
		$bind_vars['p_permission_1'] = SQ_PERMISSION_READ;
		$bind_vars['p_userid']       = $public_userid;
		$bind_vars['p_userid_1']     = $public_userid;
		$bind_vars['r_userid']       = $public_userid;
		$bind_vars['r_userid_1']     = $public_userid;
	}//end if

	// Add the User IDs bind vars
	if (!is_null($USERIDS_BIND_VARS)) {
		$userids_bind_vars_values = @unserialize($USERIDS_BIND_VARS);
		foreach ($userids_bind_vars_values as $bind_id => $bind_value) {
			$bind_vars[$bind_id] = $bind_value;
		}
	}

	$sql    = 'SELECT DISTINCT a.assetid, l.majorid, a.type_code, a.status, a.name, a.short_name, pt.path, l.sort_order
			   FROM '.SQ_TABLE_RUNNING_PREFIX.'ast a
				 INNER JOIN '.SQ_TABLE_RUNNING_PREFIX.'ast_lnk l ON a.assetid = l.minorid
				 INNER JOIN '.SQ_TABLE_RUNNING_PREFIX.'ast_path pt ON a.assetid = pt.assetid ';
	if (!empty($USERIDS_COND)) {
		$sql   .= '
				 INNER JOIN '.SQ_TABLE_RUNNING_PREFIX.'ast_perm p ON a.assetid = p.assetid ';
		$sql   .= '
				 LEFT JOIN '.SQ_TABLE_RUNNING_PREFIX.'vw_ast_role r ON p.userid = r.roleid AND p.assetid = r.assetid ';
	}
	$where  = 'l.majorid IN ('.substr($majorids_str, 0, -1).')
				 AND '.db_extras_bitand(MatrixDAL::getDbType(), 'l.link_type', SQ_SC_LINK_FRONTEND_NAV).' > 0 ';
	$where  = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'a');
	$where  = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'l');
	if (!empty($USERIDS_COND)) {
		$where  = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'p');
	}
	$where  = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'pt');
	$where .= ' '.$USERIDS_COND.'
			   ORDER BY l.majorid, l.sort_order';

	try {
		$query = MatrixDAL::preparePdoQuery($sql.$where);
		foreach ($bind_vars as $bind_var => $bind_value) {
			MatrixDAL::bindValueToPdo($query, $bind_var, $bind_value);
		}
		$result = MatrixDAL::executePdoAssoc($query);
	} catch (Exception $e) {
		throw new Exception('Unable to get valid menu entries due to database error: '.$e->getMessage());
	}

	$assetids = Array();

	foreach ($result as $row) {
		if (!($row['status'] & (SQ_STATUS_LIVE | SQ_STATUS_LIVE_APPROVAL))) {
			$menu_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($row['assetid']);
			$read_access = $menu_asset->readAccess();

			$row['name'] = $menu_asset->name;
			$row['short_name'] = $menu_asset->short_name;

			if ($row['status'] & SQ_SC_STATUS_NOT_LIVE) {
				// somewhere between under construction and live so we show this by altering the name
				$row['name']       = '(( '.$row['name'].' ))';
				$row['short_name'] = '(( '.$row['short_name'].' ))';
			}

			$GLOBALS['SQ_SYSTEM']->am->forgetAsset($menu_asset);
			unset($menu_asset);
			if (!$read_access) continue;
		}
		$assetids[] = $row['assetid'];
		if (!isset($storage[$row['majorid']])) {
			$storage[$row['majorid']] = Array();
		}
		$storage[$row['majorid']][$row['assetid']] = $row;

	}

	return $assetids;

}//end menu_get_assets()


?>
