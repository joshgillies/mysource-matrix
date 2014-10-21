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
* $Id: dependants.inc,v 1.15 2012/08/30 01:09:20 ewang Exp $
*
*/

require_once SQ_LIB_PATH.'/html_form/html_form.inc';

/**
* Dependants Static Screen Functions
*
* @author  Greg Sherwood <gsherwood@squiz.net>
* @version $Revision: 1.15 $
* @package MySource_Matrix
*/


/**
* Paints the interface for viewing dependants details
*
* @param object	&$owner		the asset whose interface we are painting
* @param object	&$o				the outputter class
* @param object	&$ei			the edit interface
* @param array	$keywords	the keywords to replace
*
* @return boolean
* @access public
*/
function paintDependants(&$owner, &$o, &$ei, $keywords=NULL)
{
	// this line has been added to hide the dependant screen from the limbo mode
	// there is some issues with the 'a' tags printing this screen in the simple edit mode
	// the links are not right and they are not working
	// a feature request should be to enable and fix this screen
	if (SQ_IN_LIMBO) return FALSE;
	$o->openSection(translate('Current Lock Status'));

	$o->openField('&nbsp;');

	$locks = $GLOBALS['SQ_SYSTEM']->am->getLockInfo($owner->id, 'all');
	$locks_acquired = Array();
	foreach ($locks as $lock_type => $lock) {
		if (!empty($lock)) {
			$locks_acquired[] = ucwords(str_replace('_', ' ', $lock_type));
		}
	}
	$lock_message = '';
	if (!empty($locks_acquired)) {
		$lock_message = sprintf(translate('%1$sLOCKED%2$s Types : %3$s'), '<span style="color:red"><b>[', ']</b></span> (', implode(', ', $locks_acquired).')');

	} else {
		$lock_message = '<span style="color:green"><b>['.translate('UNLOCKED').']</b></span>';

	}

	?>
	<table cellpadding="0" cellspacing="0" border="0" width="100%">
		<tr>
			<td class="sq-backend-data">
				<table cellpadding="0" cellspacing="0" border="0">
					<tr>
						<td class="sq-backend-data"><img src="<?php echo $o->filesPath('/images/blank.gif');?>" width="20" height="20" border="0" alt="" /></td>
						<td class="sq-backend-data" nowrap="nowrap">
							<!-- <a href="<?php echo $owner->getBackendHref('dependants'); ?>"><b><?php echo $owner->name; ?></b></a> -->
							<?php echo get_asset_tag_line($owner->id, 'dependants'); ?>
						</td>
					</tr>
				</table>
			</td>
			<td><img src="<?php echo $o->filesPath('/images/blank.gif');?>" width="5" height="1" border="0" alt="" /></td>
			<td class="sq-backend-data" nowrap="nowrap">
				<?php echo $lock_message; ?>
			</td>
			<td background="<?php echo $o->filesPath('/images/tree/stalk.gif'); ?>"><img src="<?php echo $o->filesPath('/images/blank.gif');?>" width="20" height="1" border="0" alt="" /></td>
			<td class="sq-backend-data" colspan="3" nowrap="nowrap">
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
				<?php echo sprintf(translate('The status of the dependent asset is higher than the status of "%s"'), htmlspecialchars($owner->name, ENT_COMPAT, SQ_CONF_DEFAULT_CHARACTER_SET)); ?>

			</td>
		</tr>
		<tr>
			<td class="sq-backend-data"><span style="color:red"><b>**</b></span></td>
			<td class="sq-backend-data">
				<?php echo sprintf(translate('The status of the dependent asset is lower than the status of "%s"'), htmlspecialchars($owner->name, ENT_COMPAT, SQ_CONF_DEFAULT_CHARACTER_SET)); ?>

			</td>
		</tr>
	</table>
	<?php

	$o->closeSection();

	return FALSE;

}//end paintDependants()


/**
* Paints an asset dependancy tree with stalks and dependant info
*
* @param object	&$owner				the asset whose interface we are painting
* @param object	&$asset				the asset whose dependants we are painting
* @param object	&$o					the outputter class
* @param int	$levels				how many levels down are we
* @param array	$print_stalks_at	what levels to print stalks at, an array of integer
*
* @return void
* @access public
*/
function _recursePaintDependants(&$owner, &$asset, &$o, $levels=1, $print_stalks_at=Array())
{
	$dependant_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($asset->id, SQ_SC_LINK_SIGNIFICANT, '', TRUE, 'major', NULL, 1);
	$num_kids = count($dependant_links);
	for ($i = 0; $i < $num_kids; $i++) {
		$dep_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($dependant_links[$i]['minorid'], $dependant_links[$i]['minor_type_code']);
		$end = ($i == $num_kids - 1);
		$stalk = 'background="'.$o->filesPath('/images/tree/stalk.gif').'"';
		$bg = ($end) ? '' : $stalk;

		$locks = $GLOBALS['SQ_SYSTEM']->am->getLockInfo($dep_asset->id, 'all');
		$locks_acquired = Array();
		foreach ($locks as $lock_type => $lock) {
			if (!empty($lock)) {
				$locks_acquired[] = ucwords(str_replace('_', ' ', $lock_type));
			}
		}
		$lock_message = '';
		if (!empty($locks_acquired)) {
			$lock_message = sprintf(translate('%1$sLOCKED%2$s Types : %3$s'), '<span style="color:red"><b>[', ']</b></span> (', implode(', ', $locks_acquired).')');

		} else {
			$lock_message = '<span style="color:green"><b>['.translate('UNLOCKED').']</b></span>';

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
			<td class="sq-backend-data">
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
							<td class="sq-backend-data" <?php echo $stalk_bg; ?>><img src="<?php echo $o->filesPath('/images/blank.gif'); ?>" width="20" height="1" alt="blank" /></td>
							<?php
						}
						?>
						<td class="sq-backend-data" <?php echo $bg; ?>><img src="<?php echo $o->filesPath('/images/tree/branch.gif');?>" width="20" height="20" border="0" alt="branch" /></td>
						<td class="sq-backend-data" nowrap="nowrap">
							&nbsp;
							<?php
							echo get_asset_tag_line($dependant_links[$i]['minorid'], 'dependants'); ?>
							<!-- <a href="<?php echo $dep_asset->getBackendHref('dependants'); ?>"><b><?php echo $dep_asset->name; ?></b></a> -->
						</td>
					</tr>
				</table>
			</td>
			<td><img src="<?php echo $o->filesPath('/images/blank.gif');?>" width="5" height="1" border="0" alt="" /></td>
			<td class="sq-backend-data" nowrap="nowrap">
				<?php echo $lock_message; ?>
			</td>
			<td background="<?php echo $o->filesPath('/images/tree/stalk.gif'); ?>"><img src="<?php echo $o->filesPath('/images/blank.gif');?>" width="20" height="1" border="0" alt="" /></td>
			<td class="sq-backend-data" nowrap="nowrap">
				<?php echo $dep_asset->getStatusDescription(); ?>
			</td>
			<td><img src="<?php echo $o->filesPath('/images/blank.gif');?>" width="5" height="1" border="0" alt="" /></td>
			<td class="sq-backend-data" nowrap="nowrap">
				<?php echo $status_message; ?>
			</td>
		</tr>
		<?php
		$child_deps = $GLOBALS['SQ_SYSTEM']->am->getLinks($dep_asset->id, SQ_SC_LINK_SIGNIFICANT, '', TRUE, 'major', NULL, 1);

		if (!empty($bg)) $new_print_stalks_at[] = $levels;

		if (!empty($child_deps)) {
			_recursePaintDependants($owner, $dep_asset, $o, ($levels+1), $new_print_stalks_at);
		}
	}//end for

}//end _recursePaintDependants()


/**
* Processes the interface for viewing depedants
* Note that this function doesnt do anything
*
* @param object	&$owner	the asset whose interface we are painting
* @param object	&$o		the outputter class
* @param object	&$ei	the edit interface
*
* @return boolean
* @access public
*/
function processDependants(&$owner, &$o, &$ei)
{
	return FALSE;

}//end processDependants()


?>
