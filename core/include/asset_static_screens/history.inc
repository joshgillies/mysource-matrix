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
* $Id: history.inc,v 1.2 2004/01/15 00:16:45 diarandine Exp $
* $Name: not supported by cvs2svn $
*/


require_once SQ_LIB_PATH.'/html_form/html_form.inc';


/**
* Paints the interface for viewing the history of this asset
* This includes going into rollback view mode
*
* @param object	Asset					&$owner		the asset whose interface we are painting
* @param object	Backend_Outputter		&$o			the outputter class
* @param object	Asset_Edit_Interface	&$ei		the edit interface
*
* @return boolean
* @access public
*/
function paintHistory(&$owner, &$o, &$ei)
{
	$write_access = $owner->writeAccess('');

	// prefix for all the form vars
	$prefix = $owner->getPrefix();

	$db = &$GLOBALS['SQ_SYSTEM']->db;

	// note that we dont prep this query for rollback select
	// because we are selecting directly from the rollback table
	$sql = 'SELECT '.SQ_TABLE_PREFIX.'effective_from AS efrom, '.SQ_TABLE_PREFIX.'effective_to AS eto, version
			FROM '.SQ_TABLE_ROLLBACK_PREFIX.'asset
			WHERE assetid = '.$db->quote($owner->id).'
			ORDER BY '.SQ_TABLE_PREFIX.'effective_from DESC';

	$result = $db->getAll($sql);
	if (DB::isError($result)) {
		trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_WARNING);
		return Array();
	}

	$o->openSection('Version History');
	$o->openField('');

	if (SQ_ROLLBACK_VIEW) {
		?><p class="sq-backend-data"><b><i>The highlighted row in the version history is the current version of "<?php echo $owner->name; ?>" being viewed in rollback mode</i></b></p>
		<?php
	}

	hidden_field($prefix.'_rollback_version', '0');

	?>
	<table class="sq-backend-table">
		<tr>
			<td class="sq-backend-table-header">Version Duration</td>
			<td class="sq-backend-table-header">Time Since Version</td>
			<td class="sq-backend-table-header" align="center">Version</td>
			<?php
			if ($write_access) {
				?>
				<td class="sq-backend-table-header" align="center">View</td>
				<?php
			}
			?>
		</tr>
	<?php

	foreach ($result as $data) {
		$from = $data['efrom'];
		$to   = $data['eto'];

		$class = 'sq-backend-table-cell';
		$show_view_btn = true;
		if (SQ_ROLLBACK_VIEW) {
			$rollback_from = strtotime($_SESSION['sq_rollback_view']['rollback_time']);
			if ($rollback_from >= strtotime($from) && (empty($to) || strtotime($to) > $rollback_from)) {
				$class .= '-alt';
				$show_view_btn = false;
			}
		}

		if (empty($to)) {
			$to = 'present';
			$ago = 'Most Recent';
			$show_view_btn = false;
		} else {
			require_once SQ_FUDGE_PATH.'/general/datetime.inc';
			$ago = easy_time_total(time() - strtotime($from), true);
			list($fd, $ft) = explode(' ', $from);
			list($td, $tt) = explode(' ', $to);
			if ($fd == $td) $to = $tt;
			else $to = date('d/m/Y H:i:s', strtotime($to));
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
						echo submit_button($prefix.'_rollback_mode', ' View ', 'this.form.'.$prefix.'_rollback_version.value = \''.$data['version'].'\'');
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
	}

	?>
	</table>
	<?php

	$o->closeField();
	$o->closeSection();

	return false;

}//end paintHistory()


/**
* Process the interface for viewing the history of the asset
*
* This includes going into rollback view mode
*
* @param object	Asset					&$owner	the asset whose interface we are painting
* @param object	Backend_Outputter		&$o		the outputter class
* @param object	Asset_Edit_Interface	&$ei	the edit interface
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

	return true;

}//end processHistory()


?>
