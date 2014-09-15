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
* $Id: history.inc,v 1.20 2012/08/30 01:09:20 ewang Exp $
*
*/

require_once SQ_LIB_PATH.'/html_form/html_form.inc';

/**
* History Static Screen Functions
*
* @author  Greg Sherwood <gsherwood@squiz.net>
* @version $Revision: 1.20 $
* @package MySource_Matrix
*/


/**
* Paints the interface for viewing the history of this asset
* This includes going into rollback view mode
*
* @param object	&$owner	the asset whose interface we are painting
* @param object	&$o		the outputter class
* @param object	&$ei	the edit interface
* @param array	$keywords	the keywords to replace
*
* @return boolean
* @access public
*/
function paintHistory(&$owner, &$o, &$ei, $keywords=NULL)
{
	$write_access = $owner->writeAccess('');

	// prefix for all the form vars
	$prefix = $owner->getPrefix();

	$db =& $GLOBALS['SQ_SYSTEM']->db;

	// note that we dont prep this query for rollback select
	// because we are selecting directly from the rollback table
	$sql = 'SELECT sq_rb_ast.sq_eff_from as efrom, sq_rb_ast.sq_eff_to as eto, sq_rb_ast.version, sq_ast.name as updated_username, sq_rb_ast.updated_userid as updated_userid
			FROM
			    sq_rb_ast

			LEFT JOIN sq_ast ON sq_rb_ast.updated_userid = sq_ast.assetid
			WHERE
			(
			    sq_rb_ast.assetid = (:assetid)
			)

			ORDER BY sq_rb_ast.sq_eff_from  DESC
			';

	try {
		$assetid = $owner->id;
		$query = MatrixDAL::preparePdoQuery($sql);
		MatrixDAL::bindValueToPdo($query, 'assetid', $assetid);
		$result = MatrixDAL::executePdoAll($query);
	} catch (Exception $e) {
		throw new Exception('Failed to get asset history due to database error: '.$e->getMessage());
	}

	$o->openSection(translate('version_history'));
		$o->openRaw();

			if (SQ_ROLLBACK_VIEW) {
				?><p class="sq-backend-data"><b><i><?php echo sprintf(translate('history_viewed_row'), htmlspecialchars($owner->name, ENT_COMPAT, SQ_CONF_DEFAULT_CHARACTER_SET)); ?></i></b></p>
				<?php
			}

			hidden_field($prefix.'_rollback_version', '0');

			?>
			<table class="sq-backend-table">
				<tr>
					<th class="sq-backend-table-header"><?php echo translate('version_duration'); ?></th>
					<th class="sq-backend-table-header"><?php echo translate('time_since_version'); ?></th>
					<th class="sq-backend-table-header" align="center"><?php echo translate('updated_user'); ?></th>
					<th class="sq-backend-table-header" align="center"><?php echo translate('version'); ?></th>
					<?php
					if ($write_access) {
						?>
						<th class="sq-backend-table-header" align="center"><?php echo translate('view'); ?></th>
						<?php
					}
					?>
				</tr>

				<?php
				foreach ($result as $data) {
					$from = $data['efrom'];
					$to   = $data['eto'];
					$updated_username = $data['updated_username'];
					$updated_userid = $data['updated_userid'];

					$class = 'sq-backend-table-cell';
					$show_view_btn = TRUE;
					if (SQ_ROLLBACK_VIEW) {
						$rollback_from = strtotime($_SESSION['sq_rollback_view']['rollback_time']);
						if ($rollback_from >= strtotime($from) && (empty($to) || strtotime($to) > $rollback_from)) {
							$class .= '-alt';
							$show_view_btn = FALSE;
						}
					}

					if (empty($to)) {
						$to = translate('present');
						$ago = translate('most_recent');
						$show_view_btn = FALSE;
					} else {
						require_once SQ_FUDGE_PATH.'/general/datetime.inc';
						$ago = easy_time_total(time() - strtotime($from), TRUE);
						list($fd, $ft) = explode(' ', $from);
						list($td, $tt) = explode(' ', $to);
						if ($fd == $td) {
							$to = $tt;
						} else {
							$to = date('d/m/Y H:i:s', strtotime($to));
						}
					}

					?>
					<tr>
						<td class="<?php echo $class; ?>"><?php echo date('d/m/Y H:i:s', strtotime($from)); ?> - <?php echo $to; ?></td>
						<td class="<?php echo $class; ?>"><?php echo $ago; ?></td>
						<td class="<?php echo $class; ?>"><?php echo empty($updated_username) ? $updated_userid : $updated_username; ?></td>
						<td class="<?php echo $class; ?>" align="center"><?php echo $data['version']; ?></td>

						<?php
						if ($write_access) {
							?>
							<td class="<?php echo $class; ?>" align="center">
								<?php
								if ($show_view_btn) {
									echo submit_button($prefix.'_rollback_mode', translate('view'), 'this.form.'.$prefix.'_rollback_version.value = \''.$data['version'].'\'');
								} else {
									echo '&nbsp;';
								}
								?>
							</td>
							<?php
						}
						?>
					</tr>
					<?php
				}//end foreach
				?>
			</table>
			<?php
		$o->closeRaw();
	$o->closeSection();

	return FALSE;

}//end paintHistory()


/**
* Process the interface for viewing the history of the asset
*
* This includes going into rollback view mode
*
* @param object	&$owner	the asset whose interface we are painting
* @param object	&$o		the outputter class
* @param object	&$ei	the edit interface
*
* @return boolean
* @access public
*/
function processHistory(&$owner, &$o, &$ei)
{
	// prefix for all the form vars
	$prefix = $owner->getPrefix();

	if (isset($_POST[$prefix.'_rollback_mode']) && isset($_POST[$prefix.'_rollback_version'])) {
		$version = $_POST[$prefix.'_rollback_version'];
		$url = $_SERVER['PHP_SELF'].'?SQ_ACTION=rollback_view_start&assetid='.$owner->id.'&version='.$version.'&am_section=edit_asset&asset_ei_screen=details';
		$o->setRedirect($url, 'top');
	}

	return TRUE;

}//end processHistory()


?>
