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
* $Id: history.inc,v 1.13 2006/02/07 02:18:51 skim Exp $
*
*/

require_once SQ_LIB_PATH.'/html_form/html_form.inc';

/**
* History Static Screen Functions
*
* @author  Greg Sherwood <gsherwood@squiz.net>
* @version $Revision: 1.13 $
* @package MySource_Matrix
*/


/**
* Paints the interface for viewing the history of this asset
* This includes going into rollback view mode
*
* @param object	&$owner	the asset whose interface we are painting
* @param object	&$o		the outputter class
* @param object	&$ei	the edit interface
*
* @return boolean
* @access public
*/
function paintHistory(&$owner, &$o, &$ei)
{
	$write_access = $owner->writeAccess('');

	// prefix for all the form vars
	$prefix = $owner->getPrefix();

	$db =& $GLOBALS['SQ_SYSTEM']->db;

	// note that we dont prep this query for rollback select
	// because we are selecting directly from the rollback table
	$sql = 'SELECT sq_eff_from AS efrom, sq_eff_to AS eto, version
			FROM sq_rb_ast
			WHERE assetid = '.$db->quote($owner->id).'
			ORDER BY sq_eff_from DESC';

	$result = $db->getAll($sql);
	assert_valid_db_result($result);

	$o->openSection(translate('version_history'));
		$o->openRaw();

			if (SQ_ROLLBACK_VIEW) {
				?><p class="sq-backend-data"><b><i><?php echo translate('history_viewed_row', $owner->name); ?></i></b></p>
				<?php
			}

			hidden_field($prefix.'_rollback_version', '0');

			?>
			<table class="sq-backend-table">
				<tr>
					<td class="sq-backend-table-header"><?php echo translate('version_duration'); ?></td>
					<td class="sq-backend-table-header"><?php echo translate('time_since_version'); ?></td>
					<td class="sq-backend-table-header" align="center"><?php echo translate('version'); ?></td>
					<?php
					if ($write_access) {
						?>
						<td class="sq-backend-table-header" align="center"><?php echo translate('view'); ?></td>
						<?php
					}
					?>
				</tr>

				<?php
				foreach ($result as $data) {
					$from = $data['efrom'];
					$to   = $data['eto'];

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
		$url = $_SERVER['PHP_SELF'].'?SQ_ACTION=rollback_view_start&assetid='.$owner->id.'&version='.$version;
		$o->setRedirect($url, 'top');
	}

	return TRUE;

}//end processHistory()


?>
