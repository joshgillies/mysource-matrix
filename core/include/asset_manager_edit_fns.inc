<?php
require_once SQ_LIB_PATH.'/html_form/html_form.inc';

/**
* Asset_Manager_Edit_Fns
*
* Purpose
*
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Version$ - 1.0
* @package Resolve
*/
class Asset_Manager_Edit_Fns extends Resolve_Object
{

	/**
	* Holds a reference to the asset manager
	* @var object Asset_Manager
	*/
	var $am;

	/**
	* Holds a reference to the backend object
	* @var object Backend
	*/
	var $backend;

	/**
	* Constructor
	*
	* @param object Asset_Manager	&$am
	* @param object Backend			&$backend
	*/
	function Asset_Manager_Edit_Fns(&$am, &$backend)
	{
		$this->Resolve_Object();
		$this->am = &$am;
		$this->backend = &$backend;
	}

	/**
	* Prints out the interface for viewing and customising asset types
	*
	* @access public
	*/
	function paintBackend()
	{
		$o = &$this->backend->out;

		if (!isset($_GET['am_section'])) $_GET['am_section'] = '';
		$o->addFormActionGetVar('am_section', $_GET['am_section']);
		$o->addHiddenField('am_form_submitted', '1');

		switch($_GET['am_section']) {
			case 'add_asset' :
				$parent = &$this->am->getAsset($_GET['parent_assetid']);
				if (is_null($parent)) {
					trigger_error('Parent Asset #'.$_GET['parent_assetid'].' not found', E_USER_WARNING);
					break;
				}

				// make sure that we are allowed to link this type to the parent
				if (($err_msg = $parent->canLinkToType($_GET['type_code'], $_GET['link_type'])) !== true) {
					trigger_error($err_msg, E_USER_WARNING);
					break;
				}

				$o->addFormActionGetVar('parent_assetid', $_GET['parent_assetid']);
				$o->addFormActionGetVar('pos',            $_GET['pos']);
				$o->addFormActionGetVar('type_code',      $_GET['type_code']);
				$o->addFormActionGetVar('link_type',      $_GET['link_type']);

				$this->am->includeAsset($_GET['type_code']);
				$asset = new $_GET['type_code']();

				// if the form has been submitted then, process it
				// and if that is successfull, then create the link to the parent asset
				if (!empty($_POST['am_form_submitted'])) {

					// start the transaction to create the asset and initial link
					$link = Array('asset'     => &$parent,
								  'link_type' => $_GET['link_type'],
								  'value'     => '',
								  'position'  => $_GET['pos']);

					$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
					$success = $asset->processBackend($link);

					if ($success) {

						$o->addMessage(SQ_BO_MSG_NOTICE, 'New '.$this->am->_asset_types[$_GET['type_code']]['name'].' "'.$asset->name.'" created');
						$o->addMessage(SQ_BO_MSG_NOTICE, '"'.$asset->name.'" successfully linked to "'.$parent->name.'"');
						$this->am->registerAsset($asset);
						$o->addFormActionGetVar('am_section', 'edit_asset');
						$o->addFormActionGetVar('assetid', $asset->id);
						$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');

					} else {
						// the asset was not created fully or linking failed
						$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
					}
				}

				$asset->paintBackend($this->backend);

				break;

			case 'edit_asset' :

				$o->addFormActionGetVar('assetid', $_GET['assetid']);

				$asset = &$this->am->getAsset($_GET['assetid']);
				if (is_null($asset)) {
					trigger_error('Asset #'.$_GET['assetid'].' not found', E_USER_WARNING);
					break;
				}

				// if the form has been submitted then, process it
				// and if that is successfull, then create the link to the parent asset
				if (!empty($_POST['am_form_submitted']) && $asset->processBackend(Array())) {
					$o->addMessage(SQ_BO_MSG_NOTICE, '"'.$asset->name.'" Updated');
				}// end if

				$asset->paintBackend($this->backend);
				break;

			case 'duplicate' :
				$this->_dupeAsset();
				break;


			default   :
				$this->_printAssetList();

		}//end switch

	}//end paintBackend();


	/**
	* Looks after the duplication of assets and their trees, includes the interface
	*
	* @access public
	*/
	function _dupeAsset()
	{
		$o = &$this->backend->out;

		$o->addFormActionGetVar('assetid', $_GET['assetid']);
		$asset = &$this->am->getAsset($_GET['assetid']);
		if (is_null($asset)) {
			trigger_error('Asset #'.$_GET['assetid'].' not found', E_USER_WARNING);
			break;
		}



		require_once SQ_LIB_PATH.'/progress_bar/progress_bar.inc';
		$pb = new Progress_Bar($o->getCurrentLocation(), $this);
		if (!$pb->loaded) {
			$pb->setFns('_dupeInit', '_dupeInitPaint', '_dupeDone');
			$pb->setOption('auto_close', false);

			$pb->addStep('Obtain Locks', '_dupeObtainLocks');
			$pb->addStep('Duplicate', '_dupeExec');
			$pb->addStep('Release Locks', '_dupeReleaseLocks', '_dupeReleaseLocksInit');
		}

//				$vars = get_object_vars($pb);
//				unset($vars['_obj']);
//				pre_echo($vars);

		$pb->run();

		if ($pb->inPopUp()) {
			$o->openRaw();
		} else {
			$o->setHeading('Duplicate : '.$asset->name);
			$o->openSection('Options');
			$o->openField('', 'new_line');
		}

		$pb->paint();

		if ($pb->inPopUp()) {
			$o->closeRaw();
		} else {
			$o->closeSection();
			$o->commitButton();
		}

	}// end _dupeAsset()


	/**
	*/
	function _dupeInit(&$pb)
	{
		$vars = $pb->getRunningVars();
		// first time here ?
		if (empty($vars)) {
			$vars = Array(	'assetid'			=> $_GET['assetid'],
							'to_parent_assetid' => $_GET['to_parent_assetid'],
							'to_parent_pos'		=> $_GET['to_parent_pos']
						);
			$pb->setRunningVars($vars);
		}

		// they have posted what they want to duplicate ?
		if (!empty($_POST['duplicate_assetid'])) {
			pre_echo($_POST);
			$asset = &$this->am->getAsset($vars['assetid']);
			$dependant_linkids = $asset->getDependantLinkIds();
			$tree = $this->_getAllChildLinks($vars['assetid']);

//			foreach($dependant_linkids as $linkid) {
//				if (empty($_POST['duplicate_assetid']
//
//			}

		}

		return false;

	}

	function _getAllChildLinks($assetid)
	{
		if (!isset($this->_tmp['child_links'][$assetid])) {

			$db = &$GLOBALS['SQ_SYSTEM']->db;
			$sql = 'SELECT t.treeid
					FROM sq_asset_link_tree t INNER JOIN sq_asset_link l ON t.linkid = l.linkid
					WHERE l.minorid = '.$db->quote($assetid).'
					LIMIT 1;';
	//pre_echo($sql);
			$treeid = $db->getOne($sql);
			if (DB::isError($treeid)) {
				trigger_error($treeid->getMessage().'<br/>'.$treeid->getUserInfo(), E_USER_ERROR);
			}

			$current_level = strlen($treeid) / SQ_CONF_ASSET_TREE_SIZE;

			$sql = 'SELECT SUBSTRING(t.treeid FROM '.$db->quote(strlen($treeid) + 1).') as treeid,
							(CHARACTER_LENGTH(t.treeid) / '.SQ_CONF_ASSET_TREE_SIZE.') '.(($current_level) ? ' - '.$current_level : '').' as level,
							l.linkid, a.assetid, a.short_name, a.type_code, l.dependant
					FROM sq_asset_link_tree t
						  INNER JOIN sq_asset_link l ON t.linkid = l.linkid
						  INNER JOIN sq_asset a ON l.minorid = a.assetid
					WHERE t.treeid LIKE '.$db->quote($treeid.'%').'
					  AND t.treeid > '.$db->quote($treeid).'
					ORDER BY t.treeid';
	#pre_echo($sql);
			$this->_tmp['child_links'][$assetid] = $db->getAssoc($sql, false, Array(), DB_FETCHMODE_ASSOC); // need all this because of DB API
			if (DB::isError($this->_tmp['child_links'][$assetid])) {
				trigger_error($this->_tmp['child_links'][$assetid]->getMessage().'<br/>'.$this->_tmp['child_links'][$assetid]->getUserInfo(), E_USER_ERROR);
			}

		}// end if

		return $this->_tmp['child_links'][$assetid];

	}

	/**
	*/
	function _dupeInitPaint(&$pb)
	{
		$o = &$this->backend->out;
		$vars = $pb->getRunningVars();

		$asset = &$this->am->getAsset($vars['assetid']);
		$dependant_linkids = $asset->getDependantLinkIds();
		pre_echo($dependant_linkids);
		$tree = $this->_getAllChildLinks($asset->id);
		?>
		<table border="0" cellspacing="0" cellpadding="0">
			<tr>
				<td class="sq-backend-data">
					<?php check_box('duplicate_assetid', $asset->id, true, 'alert("This is a asset that you are duplicating, you cannot unset it"); this.checked = true;');?>
				</td>
				<td class="sq-backend-data" style="font-weight: bold;">
					<?php echo $asset->short_name, ' - Id #', $asset->id; ?>
				</td>
			</tr>
			</table>
		<?php

		if (!empty($tree)) {

			?>
			<script language="javascript" type="text/javascript">
				<!-- //

					var DUPLICATE_TREEIDS = ["<?php echo implode('" ,"', array_keys($tree));?>"];

					function duplicate_treeid_check(treeid) {
						var tmp_treeid = new String(treeid);

						var clicked_elem = get_form_element("duplicate_treeid[" + tmp_treeid + "]");
						// if we were checked make sure that all our parents are checked
						if (clicked_elem.checked) {
							while (tmp_treeid.length > 0) {
								var elem = get_form_element("duplicate_treeid[" + tmp_treeid + "]");
								elem.checked = true;
								tmp_treeid = tmp_treeid.substr(0, tmp_treeid.length - <?php echo SQ_CONF_ASSET_TREE_SIZE; ?>);
							}
						}
						// Change all our children to be what we are
						for(var i = 0; i < DUPLICATE_TREEIDS.length; i++) {
							if (treeid == DUPLICATE_TREEIDS[i].substr(0, treeid.length)) {
								var elem = get_form_element("duplicate_treeid[" + DUPLICATE_TREEIDS[i] + "]");
								elem.checked = clicked_elem.checked;
							}
						}

					}// end duplicate_treeid_check()

				// -->
			</script>
			<?php

			$level = 1;
			$indents = Array();
			$dependants = Array();
			$branch_img = $o->filesPath('/images/tree/branch.gif');
			$indent_start = '<td class="sq-backend-data" %';
			$indent_end   = '%><img src="'.$o->filesPath('/images/blank.gif').'" width="20" height="20" border="0" alt="blank" /></td>';

			$last_at_level = Array();
			ob_start();
			for(reset($tree); NULL !== ($treeid = key($tree)); next($tree)) {
				$prev_level = $level;
				$level      = (int) $tree[$treeid]['level'];

				$parent_treeid = substr($treeid, 0, -SQ_CONF_ASSET_TREE_SIZE);
				$last_at_level[$parent_treeid] = $treeid;

				if ($level > $prev_level) {
					array_push($indents, $parent_treeid);
				} elseif ($level < $prev_level) {
					for($x = $prev_level; $x > $level; $x--) {
						array_pop($indents);
					}
				}

//				// if it's in the dependant links array, they can't change it
//				if ($tree[$treeid]['dependant']) {
//					$checked = true;
//					$on_click = 'alert("This is a dependant asset, needed for the correct running of this asset, you cannot unset it"); this.checked = true;';
//					$style = 'style="font-weight: bold;"';
//				} else {
//					// have we submitted this before ? if so and they checked, select it again for them
//					$checked = !empty($_POST['duplicate_treeid'][$treeid]);
//					$on_click = 'duplicate_treeid_check("'.$treeid.'");';
//					$style = '';
//				}


			?>
			<table border="0" cellspacing="0" cellpadding="0">
				<tr>
			<?php

				if (!empty($indents)) {
					echo $indent_start, implode($indent_end.$indent_start, $indents), $indent_end;
				}

				?>
					<td class="sq-backend-data" <?php echo '%', $treeid, '%'; ?>><img src="<?php echo $branch_img; ?>" width="20" height="20" border="0" alt="branch" /></td>
					<td class="sq-backend-data">
					<?php 
						if ($tree[$treeid]['dependant']) {
							echo '&nbsp;';
						} else {
							// have we submitted this before ? if so and they checked, select it again for them
							check_box('duplicate_treeid['.$treeid.']', $tree[$treeid]['assetid'], !empty($_POST['duplicate_treeid'][$treeid]), 'duplicate_treeid_check("'.$treeid.'");'); 
						}
					?>
					</td>
					<td class="sq-backend-data">
					<?php 
						echo $tree[$treeid]['short_name'], ' - Id #', $tree[$treeid]['assetid']; 
					?>
					</td>
				</tr>
				</table>
			<?php
			}// end for
			$tree_html = ob_get_contents();
			ob_end_clean();

			$bg_stalks_search  = Array();
			$bg_stalks_replace = Array();
			$bg_code = 'background="'.$o->filesPath('/images/tree/stalk.gif').'"';
			for(reset($tree); NULL !== ($treeid = key($tree)); next($tree)) {
				$bg_stalks_search[] = '%'.$treeid.'%';
				// if this treeid was the last on it's level it doesn't have a stalk bg
				$bg_stalks_replace[] = (in_array($treeid, $last_at_level, true)) ? '' : $bg_code;
			}// end for

			echo str_replace($bg_stalks_search, $bg_stalks_replace, $tree_html);

		} // end if noto empty tree

	}// end _dupeInitPaint()


	/**
	*/
	function _dupeObtainLocks(&$pb)
	{
		$vars = $pb->getRunningVars();
		$vars['locks_run_num'] = empty($vars['locks_run_num']) ? 1 : ($vars['locks_run_num'] + 1);

//		pre_echo("RUN NUM : ".$vars['locks_run_num']);

		sleep(1);

		$pb->setStatusMsg('Locking'.str_repeat('.', $vars['locks_run_num']));
		$pb->setPercent($vars['locks_run_num'] / 3);
//		pre_echo("CURRENT STEP : ");
//		pre_echo($pb->_steps[$pb->_step]);
		$pb->setRunningVars($vars);
	}

	/**
	*/
	function _dupeExec(&$pb)
	{
		$vars = $pb->getRunningVars();
		$vars['run_num'] = empty($vars['run_num']) ? 1 : ($vars['run_num'] + 1);

//		pre_echo("RUN NUM : ".$vars['run_num']);

		sleep(2);

		$pb->setStatusMsg('Running'.str_repeat('.', $vars['run_num']));
		$pb->setPercent($vars['run_num'] / 5);
//		pre_echo("CURRENT STEP : ");
//		pre_echo($pb->_steps[$pb->_step]);
		$pb->setRunningVars($vars);
	}

	/**
	*/
	function _dupeReleaseLocksInit(&$pb)
	{
		$vars = $pb->getRunningVars();
		$vars['locks_run_num'] = 0;

//		pre_echo("INIT DUPE RELEASE LOCKS RUN NUM : ".$vars['locks_run_num']);

		sleep(1);

		$pb->setStatusMsg('Initialising...');
		$pb->setRunningVars($vars);
		return true;
	}

	/**
	*/
	function _dupeReleaseLocks(&$pb)
	{
		$vars = $pb->getRunningVars();
		$vars['locks_run_num'] = empty($vars['locks_run_num']) ? 1 : ($vars['locks_run_num'] + 1);

//		pre_echo("RUN NUM : ".$vars['locks_run_num']);

		sleep(1);

		$pb->setStatusMsg('Unlocking'.str_repeat('.', $vars['locks_run_num']));
		$pb->setPercent($vars['locks_run_num'] / 3);
//		pre_echo("CURRENT STEP : ");
//		pre_echo($pb->_steps[$pb->_step]);
		$pb->setRunningVars($vars);
	}


	/**
	*/
	function _dupeDone(&$pb)
	{
		$vars = $pb->getRunningVars();
//		pre_echo("ALL DONE");
//		pre_echo($vars);
	}



	/**
	* Prints out the interface for viewing and customising asset types
	*
	* @access public
	*/
	function _printAssetList()
	{
		$o = &$this->backend->out;

		/*
		if (!empty($_REQUEST['am_action'])) {

			require_once SQ_INCLUDE_PATH.'/custom_asset_type.inc';

			$o->addFormActionGetVar('am_action', 'edit');

			pre_echo($_POST);
			switch ($_REQUEST['am_action']) {
				case 'create_custom' :
					$custom = new Custom_Asset_Type();
					if ($custom->create($_REQUEST['am_new_type_code'], $_REQUEST['am_type_code'])) {
						$o->addFormActionGetVar('am_type_code', $_REQUEST['am_new_type_code']);
						$custom->paintBackend($backend);
						return;
					}
					break;
				case 'edit' :
					$o->addFormActionGetVar('am_type_code', $_REQUEST['am_type_code']);
					$custom = new Custom_Asset_Type($_REQUEST['am_type_code']);
					$custom->paintBackend($backend);
					return;
					break;
			}
		}//end if
		*/

		//$o->addCrummingLink('Asset Types', $o->getCurrentLocation());
		$o->setHeading('Assets Types', 'create');
		$o->addHiddenField('am_action');
		$o->addHiddenField('am_new_type_code');

		$o->openSection('Assets Types');
		$o->openField('', 'new_line');


		$offspring = Array();
		for (reset($this->am->_asset_types); null !== ($type_code = key($this->am->_asset_types)); next($this->am->_asset_types)) {
			$parent = $this->am->_asset_types[$type_code]['parent_type'];
			if (!isset($offspring[$parent])) $offspring[$parent] = Array();
			$offspring[$parent][] = $type_code;
		}//end for

	?>
		<script language="JavaScript" type="text/javascript">
		<!--

			var current_asset_name = '';

			function asset_checked(asset_name, customisation) {
				current_asset_name = asset_name;

				var button = get_form_element('edit_button');
				if (customisation) {
					button.value = 'Edit ' + asset_name;
				} else {
					button.value = '- - - - - - - - - - - - -';
				}
			}

			function create_custom() {

				if (get_form_element_value('am_type_code')) {

					var type_codes = new Array("<?php echo implode('", "', array_keys($this->am->_asset_types)); ?>");
					var new_type_code = '';
					var prompt_str = '';
					do {

						if (new_type_code == '') {
							prompt_str = 'Please Enter a unique Asset Type Code for your new customisation';
						} else {
							prompt_str = '"' + new_type_code + '" is already in use\nPlease enter a unique Asset Type Code';
						}

						new_type_code = prompt(prompt_str, new_type_code);
						// they hit cancel
						if (new_type_code == null) return false;

						// make sure it's in a proper format
						new_type_code = new_type_code.toLowerCase();
						new_type_code = new_type_code.replace(/ /g, '_');
						new_type_code = new_type_code.replace(/[^a-z_]/g, '');
						new_type_code = new_type_code.replace(/_+/g, '_');

					} while (array_search(type_codes, new_type_code) != null)

					set_hidden_field('am_new_type_code', new_type_code);
					set_hidden_field('am_action', 'create_custom');
					return true;
				}
				return false;
			}


		//-->
		</script>
		<table cellpadding="0" cellspacing="0" border="0">
			<tr>
				<td class="sq-backend-data"><img src="<?php echo $o->filesPath('/images/blank.gif'); ?>" width="1" height="20" border="0" alt="branch" /></td>
				<td class="sq-backend-data">
					&nbsp;Asset
				</td>
			</tr>
			<tr>
				<td class="sq-backend-data"><img src="<?php echo $o->filesPath('/images/blank.gif'); ?>" width="1" height="1" alt="blank" /></td>
				<td class="sq-backend-data">
			<?php
				$this->_recursePrintAssetList($offspring, 'asset');
			?>
				</td>
			</tr>
			<tr>
				<td class="sq-backend-data"><img src="<?php echo $o->filesPath('/images/blank.gif'); ?>" width="1" height="1" alt="blank" /></td>
				<td class="sq-backend-data">
			<?php
				//submit_button('create_button', 'Create Custom Asset', 'return create_custom();');
				//submit_button('edit_button', '- - - - - - - - - - - - -', 'if (this.value.substr(0, 4) == \'Edit\') { set_hidden_field(\'am_action\', \'edit\'); return true; } else { return false };');
			?>
				</td>
			</tr>
		</table>
	<?php

	}// end printAssetList()

	function _recursePrintAssetList(&$offspring, $parent)
	{
		$o = &$this->backend->out;

	?>
		<table cellpadding="0" cellspacing="0" border="0">
	<?php
		$num_kids = count($offspring[$parent]);
		for ($i = 0; $i < $num_kids; $i++) {

			$type_code = $offspring[$parent][$i];
			$end = ($i == $num_kids - 1);
//			$bg_img = 'images/page_'.(($page[effective_visible]) ? '' : 'in').'visible_'.page::get_status_colour($page[effective_status]).'.gif';

			$bg = ($end) ? '' : 'background="'.$o->filesPath('/images/tree/stalk.gif').'"';
		?>
			<tr>
				<td class="sq-backend-data" <?php echo $bg; ?>><img src="<?php echo $o->filesPath('/images/tree/branch.gif');?>" width="20" height="20" border="0" alt="branch" /></td>
				<td class="sq-backend-data">
				<?php /*
					<  ?php radio_button('am_type_code', $type_code, false, 'asset_checked(\''.addslashes($this->am->_asset_types[$type_code]['name']).'\', '.(($this->am->_asset_types[$type_code]['customisation']) ? 'true' : 'false').');'); ?  >
					<span class="sq-backend-<  ?php echo ($this->am->_asset_types[$type_code]['customisation']) ? 'custom-' : ''?  >asset"><  ?php echo $this->am->_asset_types[$type_code]['name'].' ('.$type_code.')';?  ></span>
					*/
				?>
					&nbsp;&nbsp;<?php echo $this->am->_asset_types[$type_code]['name'].' ('.$type_code.')';?>
				</td>
			</tr>
		<?php
			if (!empty($offspring[$type_code])) {
			?>
				<tr>
					<td class="sq-backend-data" <?php echo $bg; ?>><img src="<?php echo $o->filesPath('/images/blank.gif'); ?>" width="1" height="1" alt="blank" /></td>
					<td class="sq-backend-data">
				<?php
					$this->_recursePrintAssetList($offspring, $type_code);
				?>
					</td>
				</tr>
			<?php
			}//end if

		}//end for
	?>
		</table>
	<?php

	}//end _printAssetList()

}//end class
?>
