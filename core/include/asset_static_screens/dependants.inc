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
* $Id: dependants.inc,v 1.2 2004/01/16 11:45:56 brobertson Exp $
* $Name: not supported by cvs2svn $
*/

require_once SQ_LIB_PATH.'/html_form/html_form.inc';

/**
* Dependants Static Screen Functions
*
* @author  Greg Sherwood <gsherwood@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix
*/


/**
* Paints the interface for viewing dependants details
*
* @param object	Asset					$owner	the asset whose interface we are painting
* @param object	Backend_Outputter		$o		the outputter class
* @param object	Asset_Edit_Interface	&$ei		the edit interface
*
* @return boolean
* @access public
*/
function paintDependants(&$owner, &$o, &$ei)
{
	$o->openSection('Current Lock Status');
	$o->openField('&nbsp;');

	$locks = $GLOBALS['SQ_SYSTEM']->am->getLockInfo($owner->id, 'all');
	$locks_acquired = Array();
	foreach($locks as $lock_type => $lock) {
		if (!empty($lock)) $locks_acquired[] = ucwords(str_replace('_', ' ', $lock_type));
	}
	$lock_message = '';
	if (!empty($locks_acquired)) {
		$lock_message = '<span style="color:red"><b>[LOCKED]</b></span> (Types : '.implode(', ', $locks_acquired).')';
	} else {
		$lock_message = '<span style="color:green"><b>[UNLOCKED]</b></span>';
	}

	?>
	<table cellpadding="0" cellspacing="0" border="0" bgcolor="#E6E0EA" width="100%">
		<tr>
			<td class="sq-backend-data" style="background-color:#E6E0EA">
				<table cellpadding="0" cellspacing="0" border="0">
					<tr>
						<td class="sq-backend-data" style="background-color:#E6E0EA"><img src="<?php echo $o->filesPath('/images/blank.gif');?>" width="20" height="20" border="0" alt="" /></td>
						<td class="sq-backend-data" nowrap style="background-color:#E6E0EA">
							<a href="<?php echo $owner->getBackendHref('dependants'); ?>"><b><?php echo $owner->name; ?></b></a>
						</td>
					</tr>
				</table>
			</td>
			<td><img src="<?php echo $o->filesPath('/images/blank.gif');?>" width="5" height="1" border="0" alt="" /></td>
			<td class="sq-backend-data" valign="top" nowrap style="background-color:#E6E0EA">
				<?php echo $lock_message; ?>
			</td>
			<td background="<?php echo $o->filesPath('/images/tree/stalk.gif'); ?>"><img src="<?php echo $o->filesPath('/images/blank.gif');?>" width="20" height="1" border="0" alt="" /></td>
			<td class="sq-backend-data" colspan="3" nowrap style="background-color:#E6E0EA">
				<?php echo $owner->getStatusDescription(); ?>
			</td>
			<td width="100%">&nbsp;</td>
		</tr>
		<?php _recursePaintDependants($owner, $owner, $o); ?>
	</table>
	<table>
		<tr>
			<td class="sq-backend-data"><span style="color:red"><b>*</b></span></td>
			<td class="sq-backend-data">
				The status of the dependant asset is higher than the status of "<?php echo $owner->name; ?>"
			</td>
		</tr>
		<tr>
			<td class="sq-backend-data"><span style="color:red"><b>**</b></span></td>
			<td class="sq-backend-data">
				The status of the dependant asset is lower than the status of "<?php echo $owner->name; ?>"
			</td>
		</tr>
	</table>
	<?php

	$o->closeSection();

	return false;

}//end paintDependants()


/**
* Paints an asset dependancy tree with stalks and dependant info
*
* @param object	Asset				$owner				the asset whose interface we are painting
* @param object	Asset				$asset				the asset whose dependants we are painting
* @param object	Backend_Outputter	$o					the outputter class
* @param int						$levels				how many levels down are we
* @param array(int)					$print_stalks_at	what levels to print stalks at
*
* @return void
* @access public
*/
function _recursePaintDependants(&$owner, &$asset, &$o, $levels=1, $print_stalks_at=Array())
{
	$dependant_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($asset->id, SQ_SC_LINK_SIGNIFICANT, '', true, 'major', null, 1);
	$num_kids = count($dependant_links);
	for ($i = 0; $i < $num_kids; $i++) {
		$dep_asset = &$GLOBALS['SQ_SYSTEM']->am->getAsset($dependant_links[$i]['minorid'], $dependant_links[$i]['minor_type_code']);
		$end = ($i == $num_kids - 1);
		$stalk = 'background="'.$o->filesPath('/images/tree/stalk.gif').'"';
		$bg = ($end) ? '' : $stalk;

		$locks = $GLOBALS['SQ_SYSTEM']->am->getLockInfo($dep_asset->id, 'all');
		$locks_acquired = Array();
		foreach($locks as $lock_type => $lock) {
			if (!empty($lock)) $locks_acquired[] = ucwords(str_replace('_', ' ', $lock_type));
		}
		$lock_message = '';
		if (!empty($locks_acquired)) {
			$lock_message = '<span style="color:red"><b>[LOCKED]</b></span> (Types : '.implode(', ', $locks_acquired).')';
		} else {
			$lock_message = '<span style="color:green"><b>[UNLOCKED]</b></span>';
		}

		$status_message = '';
		if ($dep_asset->status < $owner->status) {
			$status_message = '<span style="color:red"><b>**</b></span>';
		} else if ($dep_asset->status > $owner->status) {
			$status_message = '<span style="color:red"><b>*</b></span>';
		}

		$new_print_stalks_at = Array();

		?>
		<tr>
			<td class="sq-backend-data" style="background-color:#E6E0EA">
				<table cellpadding="0" cellspacing="0" border="0">
					<tr>
						<?php
						for ($x = 0; $x < $levels; $x++) {
							$stalk_bg = '';
							if (in_array($x, $print_stalks_at)) {
								$stalk_bg = $stalk;
								$new_print_stalks_at[] = $x;
							}
							?>
							<td class="sq-backend-data" <?php echo $stalk_bg; ?> style="background-color:#E6E0EA"><img src="<?php echo $o->filesPath('/images/blank.gif'); ?>" width="20" height="1" alt="blank" /></td>
							<?php
						}
						?>
						<td class="sq-backend-data" <?php echo $bg; ?> style="background-color:#E6E0EA"><img src="<?php echo $o->filesPath('/images/tree/branch.gif');?>" width="20" height="20" border="0" alt="branch" /></td>
						<td class="sq-backend-data" nowrap style="background-color:#E6E0EA">
							&nbsp;<a href="<?php echo $dep_asset->getBackendHref('dependants'); ?>"><b><?php echo $dep_asset->name; ?></b></a>
						</td>
					</tr>
				</table>
			</td>
			<td><img src="<?php echo $o->filesPath('/images/blank.gif');?>" width="5" height="1" border="0" alt="" /></td>
			<td class="sq-backend-data" nowrap style="background-color:#E6E0EA">
				<?php echo $lock_message; ?>
			</td>
			<td background="<?php echo $o->filesPath('/images/tree/stalk.gif'); ?>"><img src="<?php echo $o->filesPath('/images/blank.gif');?>" width="20" height="1" border="0" alt="" /></td>
			<td class="sq-backend-data" valign="top" nowrap style="background-color:#E6E0EA">
				<?php echo $dep_asset->getStatusDescription(); ?>
			</td>
			<td><img src="<?php echo $o->filesPath('/images/blank.gif');?>" width="5" height="1" border="0" alt="" /></td>
			<td class="sq-backend-data" nowrap style="background-color:#E6E0EA">
				<?php echo $status_message; ?>
			</td>
		</tr>
		<?php
		$child_deps = $GLOBALS['SQ_SYSTEM']->am->getLinks($dep_asset->id, SQ_SC_LINK_SIGNIFICANT, '', true, 'major', null, 1);

		if (!empty($bg)) $new_print_stalks_at[] = $levels;

		if (!empty($child_deps)) {
			_recursePaintDependants($owner, $dep_asset, $o, ($levels+1), $new_print_stalks_at);
		}
	}

}//end _recursePaintDependants


/**
* Processes the interface for viewing depedants
* Note that this function doesnt do anything
*
* @param object	Asset					&$owner		the asset whose interface we are painting
* @param object	Backend_Outputter		&$o			the outputter class
* @param object	Asset_Edit_Interface	&$ei		the edit interface
*
* @return boolean
* @access public
*/
function processDependants(&$owner, &$o, &$ei)
{
	return false;

}//end processDependants()


?>
