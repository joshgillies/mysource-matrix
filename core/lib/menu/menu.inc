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
* $Id: menu.inc,v 1.1 2006/09/26 04:22:58 amiller Exp $
*
*/

function menu_get_assets($id_name, $majorids) {

	$db = &$GLOBALS['SQ_SYSTEM']->db;

	if (!isset($GLOBALS['MENU_DATA'][$id_name])) {
		$GLOBALS['MENU_DATA'][$id_name] = Array();
	}
	$storage =& $GLOBALS['MENU_DATA'][$id_name];
	$assetids = Array();
	$majorids_str = '';
	foreach ($majorids as $id) {
		if (empty($storage[$id])) {
			$majorids_str .= $db->quote((String)$id).',';
		} else {
			foreach ($storage[$id] as $row) {
				$assetids[] = (int) $row['assetid'];
			}
		}
	}

	if (empty($majorids_str)) return $assetids;

	static $USERIDS_COND = NULL;

	if (is_null($USERIDS_COND)) {
		// if they are logged in add their parents to the list
		if ($GLOBALS['SQ_SYSTEM']->userRoot() || $GLOBALS['SQ_SYSTEM']->userSystemAdmin()) {
			$USERIDS_COND = '';
		} else {
			$userids = array_keys($GLOBALS['SQ_SYSTEM']->am->getParents($GLOBALS['SQ_SYSTEM']->user->id, 'user_group', FALSE));
			$public_userid = $GLOBALS["SQ_SYSTEM"]->am->getSystemAssetid("public_user");
			$userids[] = $public_userid;
			$userids[] = $GLOBALS["SQ_SYSTEM"]->user->id;
			for (reset($userids); NULL !== ($i = key($userids)); next($userids)) {
				$userids[$i] = $db->quote((String)$userids[$i]);
			}
			$userids_str = implode(',', $userids);
			$USERIDS_COND = 'AND (p.userid  IN ('.$userids_str.') ';
			$USERIDS_COND .= 'OR r.userid  IN ('.$userids_str.')) ';
			$USERIDS_COND .= ' AND (
				(p.permission = '.$db->quote((Integer)SQ_PERMISSION_READ).' AND (
						p.userid <> '.$db->quote($public_userid).'
						OR r.userid <> '.$db->quote($public_userid).'
						OR (p.userid = '.$db->quote($public_userid).' AND p.granted = \'1\')
						OR (r.userid = '.$db->quote($public_userid).' AND p.granted = \'1\')
					)
				)
				OR	(
						p.permission > '.$db->quote((Integer)SQ_PERMISSION_READ).' AND p.granted = \'1\'
					)
			)';
		}

		if (!empty($USERIDS_COND)) {
			$USERIDS_COND .= '
							  GROUP BY a.assetid, l.majorid, a.type_code, a.status, a.name, a.short_name, pt.path, l.sort_order, p.assetid
							  HAVING MIN(p.granted) <> \'0\'';
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
				 LEFT JOIN '.SQ_TABLE_RUNNING_PREFIX.'vw_ast_role r ON p.userid = r.roleid ';
	}
	$where  = 'l.majorid IN ('.substr($majorids_str, 0, -1).')
				 AND '.db_extras_bitand($db, 'l.link_type', SQ_SC_LINK_FRONTEND_NAV).' > 0 ';
	$where  = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'a');
	$where  = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'l');
	if (!empty($USERIDS_COND)) {
		$where  = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'p');
	}
	$where  = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'pt');
	$where .= ' '.$USERIDS_COND.'
			   ORDER BY l.majorid, l.sort_order';

	$result = $db->query($sql.$where);
	assert_valid_db_result($result);

	$assetids = Array();
	while (DB_OK === $result->fetchInto($row)) {
		if (!($row['status'] & (SQ_STATUS_LIVE | SQ_STATUS_LIVE_APPROVAL))) {
			$menu_asset = &$GLOBALS['SQ_SYSTEM']->am->getAsset($row['assetid']);
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
		if (!isset($storage[$row['majorid']])) $storage[$row['majorid']] = Array();
		$storage[$row['majorid']][$row['assetid']] = $row;

	}
	$result->free();

	return $assetids;

}// end menu_get_assets()

?>