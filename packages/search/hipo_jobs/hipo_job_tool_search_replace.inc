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
* $Id: hipo_job_tool_search_replace.inc,v 1.23 2012/08/30 00:59:15 ewang Exp $
*
*/

require_once SQ_SYSTEM_ROOT.'/core/hipo/hipo_job.inc';

/**
* HIPO_Job_Tool_Search_Replace
*
* Purpose
*    Highly Intensive Processing Object (HIPO) to perform search and
*    replace on asset attributes and metadata
*
* @author  Nathan de Vries <ndvries@squiz.net>
* @version $Revision: 1.23 $
* @package MySource_Matrix_Packages
* @subpackage search
*/
class HIPO_Job_Tool_Search_Replace extends HIPO_Job
{


	/**
	* Constructor
	*
	* @param string	$code_name	a unique codename the HIPO
	*/
	function HIPO_Job_Tool_Search_Replace($code_name='')
	{
		$this->_hipo_vars['job_dir'] = SQ_PACKAGES_PATH.'/packages/search/tools/tool_search_replace';
		$GLOBALS['SQ_SYSTEM']->lm->includeAssetStrings('tool_search_replace');
		$this->HIPO_Job($code_name);

	}//end constructor


	/**
	* Returns a unique codename the HIPO can use to ensure it is not being run twice
	*
	* @return string
	* @access public
	*/
	function getCodeName()
	{
		return parent::getCodeName().'-'.md5(implode('-',array_keys($this->_running_vars['search_data'])));

	}//end getCodeName()


	/**
	* Returns the (localised) HIPO name
	*
	* @return string
	* @access public
	*/
	function getHipoName()
	{
		return translate('sch_hipo_name_global_sr');

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
					'name'			=> translate('hipo_confirmation'),
					'function_call'	=> Array(
										'paint_function'	=> 'paintConfirmation',
										'process_function'	=> 'processConfirmation',
									   ),
					'running_mode'	=> 'web',
					'skip_step'		=> FALSE,
					'auto_step'		=> FALSE,
					'allow_cancel'	=> TRUE,
					'percent_done'	=> 0,
					'complete'		=> FALSE,
					'message'		=> '',
				),
				Array(
					'name'			=> translate('hipo_acquiring_locks'),
					'hipo_job'		=> Array(
										'init_details_function'	=> 'getLockAssetDetails',
									   ),
					'running_mode'	=> 'server',
					'auto_step'		=> TRUE,
					'allow_cancel'	=> FALSE,
					'percent_done'	=> 0,
					'complete'		=> FALSE,
					'message'		=> '',
				),
				Array(
					'name'			=> translate('sch_hipo_replacing_asset_attributes'),
					'function_call'	=> Array(
										'process_function'	=> 'processAttributes',
									   ),
					'running_mode'	=> 'server',
					'skip_step'		=> FALSE,
					'auto_step'		=> TRUE,
					'allow_cancel'	=> TRUE,
					'percent_done'	=> 0,
					'complete'		=> FALSE,
					'message'		=> '',
				),
				Array(
					'name'			=> translate('sch_hipo_replacing_asset_contents'),
					'function_call'	=> Array(
										'process_function'	=> 'processContent',
									   ),
					'running_mode'	=> 'server',
					'skip_step'		=> FALSE,
					'auto_step'		=> TRUE,
					'allow_cancel'	=> TRUE,
					'percent_done'	=> 0,
					'complete'		=> FALSE,
					'message'		=> '',
				),
				Array(
					'name'			=> translate('sch_hipo_replacing_asset_metadata'),
					'function_call'	=> Array(
										'process_function'	=> 'processMetadata',
									   ),
					'running_mode'	=> 'server',
					'skip_step'		=> FALSE,
					'auto_step'		=> TRUE,
					'allow_cancel'	=> TRUE,
					'percent_done'	=> 0,
					'complete'		=> FALSE,
					'message'		=> '',
				),
				Array(
					'name'			=> translate('sch_hipo_replacing_schema_values'),
					'function_call'	=> Array(
										'process_function'	=> 'processSchemas',
									   ),
					'running_mode'	=> 'server',
					'skip_step'		=> FALSE,
					'auto_step'		=> TRUE,
					'allow_cancel'	=> TRUE,
					'percent_done'	=> 0,
					'complete'		=> FALSE,
					'message'		=> '',
				),
				Array(
					'name'			=> translate('hipo_releasing_locks'),
					'function_call'	=> Array(
										'process_function'	=> 'releaseLocks',
									   ),
					'running_mode'	=> 'server',
					'auto_step'		=> TRUE,
					'skip_step'		=> FALSE,
					'allow_cancel'	=> FALSE,
					'percent_done'	=> 0,
					'complete'		=> FALSE,
					'message'		=> '',
				),
				Array(
					'name'			=> translate('sch_hipo_regenerating_metadata'),
					'hipo_job'		=> Array(
										'init_details_function'	=> 'getRegenerateMetadataDetails',
									   ),
					'running_mode'	=> 'server',
					'auto_step'		=> TRUE,
					'allow_cancel'	=> FALSE,
					'percent_done'	=> 0,
					'complete'		=> FALSE,
					'message'		=> '',
				),
				Array(
					'name'			=> translate('sch_hipo_reindexing_assets'),
					'hipo_job'		=> Array(
										'init_details_function'	=> 'getReindexAssetDetails',
									   ),
					'running_mode'	=> 'server',
					'auto_step'		=> TRUE,
					'allow_cancel'	=> FALSE,
					'percent_done'	=> 0,
					'complete'		=> FALSE,
					'message'		=> '',
				),
			   );

	}//end getInitialStepData()


	/**
	* Prepares the running vars and gets the children of the asset in question
	*
	* @return boolean
	* @access public
	*/
	function prepare()
	{
		if (array_key_exists('contextid', $this->_running_vars) === FALSE) {
			trigger_localised_error('SCH0031', E_USER_WARNING);
			return FALSE;
		}

		return parent::prepare();

	}//end prepare()


	/**
	* This function is used to set-up the details for locking the assets by the nested HIPO_Job
	*
	* @param string	&$job_type		Job type
	* @param array	&$running_vars	Job vars
	* @param array	&$options		Options
	*
	* @return void
	* @access public
	*/
	function getLockAssetDetails(&$job_type, &$running_vars, &$options)
	{
		$job_type = 'hipo_job_acquire_locks';

		$options['auto_complete'] = TRUE;

		$running_vars = Array(
							'assetids'			=> array_keys($this->_running_vars['search_data']),
							'lock_type'			=> Array('all'),
							'forceably_acquire'	=> FALSE,
							'dependants_only'	=> TRUE,
						);

	}//end getLockAssetDetails()


	/**
	* This function is used to set-up the details for the regenerate metadata nested HIPO_Job
	*
	* @param string	&$job_type		Job type
	* @param array	&$running_vars	Job vars
	* @param array	&$options		Options
	*
	* @return void
	* @access public
	*/
	function getRegenerateMetadataDetails(&$job_type, &$running_vars, &$options)
	{
		$job_type = 'hipo_job_regenerate_metadata';

		$options['auto_complete'] = TRUE;

		$running_vars = Array(
							'schemaids'		=> $this->_running_vars['schemaids'],
							'contextids'	=> Array($this->_running_vars['contextid']),
						);

	}//end getRegenerateMetadataDetails()


	/**
	* This function is used to set-up the details for the reindex assets nested HIPO_Job
	*
	* @param string	&$job_type		Type of job
	* @param array	&$running_vars	Job vars
	* @param array	&$options		Options
	* @param array	&$hipo_vars		Hipo vars
	*
	* @return void
	* @access public
	*/
	function getReindexAssetDetails(&$job_type, &$running_vars, &$options, &$hipo_vars)
	{
		$job_type = 'hipo_job_reindex';

		$options['auto_complete'] = TRUE;

		$running_vars = Array(
							'root_assetid'	=> array_keys($this->_running_vars['search_data']),
							'contextid'		=> $this->_running_vars['contextid'],
							'root_only'		=> TRUE,
						);

		$hipo_vars = Array(
						'job_dir'	=> SQ_PACKAGES_PATH.'/search/hipo_jobs',
					 );

	}//end getReindexAssetDetails()


	/**
	* Release the locks that were acquired while cloning assets
	*
	* @param array	&$step_data	a reference to the array of information about the current step
	* @param string	$prefix		prefix for form vars
	*
	* @return boolean
	* @access public
	*/
	function releaseLocks(&$step_data, $prefix)
	{
		if (!isset($this->_running_vars['re_lock_assets'])) {
			$this->_running_vars['re_lock_assets'] = array_keys($this->_running_vars['search_data']);
		}

		$assetid = array_shift($this->_running_vars['re_lock_assets']);
		unset($this->_running_vars['re_lock_assets'][$assetid]);

		$children = $GLOBALS['SQ_SYSTEM']->am->getDependantChildren($assetid);

		foreach ($children as $dep_child_id => $dep_child_type_code) {
			$GLOBALS['SQ_SYSTEM']->am->releaseLock($dep_child_id, 'all');
		}

		$asset_info = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo(Array($assetid));

		$step_data['message'] = translate('sch_tool_global_sr_releasing_locks', $asset_info[$assetid]['name']);

		$GLOBALS['SQ_SYSTEM']->am->releaseLock($assetid, 'all');

		$this->_running_vars['done_assetids'][] = $assetid;

		if (empty($this->_running_vars['re_lock_assets'])) {
			unset($this->_running_vars['done_assetids']);
			$step_data['percent_done'] = 100;
			$step_data['complete']     = TRUE;
		} else {
			$step_data['percent_done'] = (count($this->_running_vars['done_assetids']) / count(array_keys($this->_running_vars['search_data']))) * 100;
			$step_data['complete']     = FALSE;
		}

		return TRUE;

	}//end releaseLocks()


	/**
	* Get the search count
	*
	* @return void
	* @access public
	*/
	function _getSearchCounts()
	{
		$search_count = Array();
		$search_string = preg_replace('/([\.\\\!\+\*\?\[\]\^\$\(\)\=\!\<\>\|\:\/]+)/i', '\\\${1}', $this->_running_vars['search_string']);

		foreach ($this->_running_vars['search_data'] as $assetid => $contents) {
			$search_count[$assetid] = 0;

			foreach (Array('attributes', 'contents', 'metadata', 'schema') as $type) {
				if (isset($contents[$type])) {

					foreach ($contents[$type] as $index => $value) {

						if ($type == 'metadata' || $type == 'schema') {
							$value = $value['value'];
						}
						if(is_array($value)) continue;
						
						$matches = Array();
						preg_match_all("/$search_string/i", $value, $matches);
						$search_count[$assetid] += count($matches[0]);
						$cvrd_sch_str = htmlentities($search_string);
						if ($cvrd_sch_str != $search_string) {
							$matches = Array();
							preg_match_all('/'.$cvrd_sch_str.'/i', $value, $matches);
							$search_count[$assetid] += count($matches[0]);
						}
						unset($matches);
					}
				}
			}

		}
		return $search_count;

	}//end _getSearchCounts()


	/**
	* Paints some option for the user to specify what replacements are to be made
	* as a result of the search
	*
	* @param array	&$step_data	a reference to the step data for this step
	* @param object	&$o			the hipo backend outputter object
	* @param string	$prefix		a string for prefixing form variables
	*
	* @return boolean
	* @access public
	*/
	function paintConfirmation(&$step_data, &$o, $prefix)
	{
		$GLOBALS['SQ_SYSTEM']->changeContext($this->_running_vars['contextid']);

		$search_count = $this->_getSearchCounts();
		$this->_setupMatches();
		$this->save();
		$o->openSection(translate('sch_tool_global_sr_confirmation'));
		$o->openField('', 'replace_selection');

		?>
		<script type="text/javascript" src="<?php echo sq_web_path('data'); ?>/asset_types/tool_search_replace/js/search_and_replace.js"></script>

		<style type="text/css">
			@import url(<?php echo sq_web_path('data'); ?>/asset_types/tool_search_replace/css/search_and_replace.css);
			table.expandable thead th {
				background-image: url(<?php echo sq_web_path('lib'); ?>/web/images/icons/expand_arrow.gif);
			}
			table thead th.match-checkbox, table thead th.all-asset-checkbox {
				background-image: none;
			}
		</style>
		<p style="text-align: center">
			<span class="link" id="select-deselect-link-top" onclick="setAllCheckboxes(this)"><?php echo translate('sch_tool_global_sr_select_all'); ?></span> &nbsp;&nbsp;&nbsp;|&nbsp;&nbsp;&nbsp;
			<span class="link" id="expand-collapse-link-top" onclick="toggleAllTbodyDisplays(this)" ><?php echo translate('sch_tool_global_sr_expand_all'); ?></span>
		</p>

		<div id="confirmations-container">
		<?php

		$names = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo(array_keys($this->_running_vars['search_data']), '', FALSE, 'name');
		foreach ($this->_running_vars['search_data'] as $assetid => $value) {
			?>
			<table class="sq-backend-table expandable" style="margin-bottom: 10px">
				<thead>
					<tr>
						<th onclick="toggleTBodyDisplay(this)">
							<?php echo get_asset_tag_line($assetid); ?>
							- <?php echo translate('sch_tool_global_sr_file_match(es)', $search_count[$assetid]); ?>
						</th>
						<th class="all-asset-checkbox">
							<?php check_box($prefix.'_asset_'.$assetid.'_all', 1, FALSE, 'toggleTBodyCheckboxes(this)'); ?>
						</th>
					</tr>
				</thead>
				<tbody style="display: none">
					<tr>
						<td colspan="2">
						<?php
						// ATTRIBUTES
						if (!empty($value['attributes'])) {
							$this->_printAttributeMatches($assetid, $prefix, $value['attributes']);
						}

						// METADATA
						if (!empty($value['metadata'])) {
							$this->_printMetadataMatches($assetid, $prefix, $value['metadata']);
						}

						// CONTENTS
						if (isset($value['contents'])) {
							$this->_printContentMatches($assetid, $prefix, $value['contents']);
						}

						// SCHEMA
						if (isset($value['schema'])) {
							$this->_printSchemaMatches($assetid, $prefix, $value['schema']);
						}
						?>
						</td>
					</tr>
				</tbody>
			</table>
			<?php

		}//end foreach asset

		?>
		</div>
		<p style="text-align: center">
			<span class="link" id="select-deselect-link-bottom" onclick="setAllCheckboxes(this)"><?php echo translate('sch_tool_global_sr_select_all'); ?></span> &nbsp;&nbsp;&nbsp;|&nbsp;&nbsp;&nbsp;
			<span class="link" id="expand-collapse-link-bottom" onclick="toggleAllTbodyDisplays(this)" ><?php echo translate('sch_tool_global_sr_expand_all'); ?></span>
		</p>
		<?php

		$o->addHiddenField('form_submitted', 1);
		$o->closeField();
		$o->closeSection();

		$GLOBALS['SQ_SYSTEM']->restoreContext();

		return TRUE;

	}//end paintConfirmation()


	/**
	* Processes the confirmation/match selection step of the HIPO
	*
	* @param array	&$step_data	a reference to the step data for this step
	* @param string	$prefix		a string for prefixing form variables
	*
	* @return boolean
	* @access public
	*/
	function processConfirmation(&$step_data, $prefix)
	{
		if (!isset($_REQUEST['form_submitted'])) {
			// No submission yet, need to print form;
			return TRUE;
		}
		// Form has been submitted, now we can process it
		$this->_updateSearchData($prefix);
		$this->save();
		if (empty($this->_running_vars['search_data'])) {
			$this->skipStep(1);
			$this->skipStep(2);
			$this->skipStep(3);
			$this->skipStep(4);
			$this->skipStep(5);
			$this->skipStep(6);
			$this->skipStep(7);
			$this->skipStep(8);
		} else {
			$this->_running_vars['attribute_replacements_total'] = 0;
			$this->_running_vars['content_replacements_total'] = 0;
			$this->_running_vars['metadata_replacements_total'] = 0;
			$this->_running_vars['schema_replacements_total'] = 0;

			foreach ($this->_running_vars['search_data'] as $assetid => $value) {
				if (isset($value['attributes'])) {
					$this->_running_vars['attribute_replacements'][$assetid] = $value['attributes'];
					$this->_running_vars['attribute_replacements_total']++;
				}
				if (isset($value['contents'])) {
					$this->_running_vars['content_replacements'][$assetid] = $value['contents'];
					$this->_running_vars['content_replacements_total']++;
				}
				if (isset($value['metadata'])) {
					$this->_running_vars['metadata_replacements'][$assetid] = $value['metadata'];
					$this->_running_vars['metadata_replacements_total']++;
				}
				if (isset($value['schema'])) {
					$this->_running_vars['schema_replacements'][$assetid] = $value['schema'];
					$this->_running_vars['schema_replacements_total']++;
					$this->_running_vars['schemaids'][] = $assetid;
				}
			}
			if ($this->_running_vars['attribute_replacements_total'] == 0) {
				$this->skipStep(2);
			}
			if ($this->_running_vars['content_replacements_total'] == 0) {
				$this->skipStep(3);
			}
			if ($this->_running_vars['metadata_replacements_total'] == 0) {
				$this->skipStep(4);
			}
			if ($this->_running_vars['schema_replacements_total'] == 0) {
				$this->skipStep(5);
				$this->skipStep(7);
			}
		}//end else

		$step_data['percent_done'] = 100;
		$step_data['complete'] = TRUE;

		return TRUE;

	}//end processConfirmation()


	/**
	* Print matches from an asset's attributes
	*
	* @param string	$assetid		The assets id
	* @param string	$prefix			Prefix for the fields
	* @param array	&$attr_matches	Matches
	*
	* @return void
	* @access private
	*/
	function _printAttributeMatches($assetid, $prefix, &$attr_matches)
	{
		?>
		<table class="sq-backend-table expandable component" border="1">
			<thead>
				<tr>
					<th colspan="2" onclick="toggleTBodyDisplay(this)">
						<?php echo translate('asset_attributes'); ?>
					</th>
					<th style="text-align: right" class="match-checkbox">
						<?php check_box($prefix.'_asset_'.$assetid.'_attr_all', 1, FALSE, 'toggleTBodyCheckboxes(this); updateParentCheckboxes(this);', 'class="component-checkbox"'); ?>
					</th>

				</tr>
			</thead>
			<tbody style="display: none">
				<tr>
					<th style="width: 30%"><?php echo translate('attribute_name'); ?></th>
					<th><?php echo translate('match'); ?></th>
					<th class="match-checkbox">&nbsp;</th>
				</tr>
			<?php
			foreach ($attr_matches as $attr_name => $attr_details) {
				foreach ($attr_details['matches'] as $offset) {
					$html_id = $prefix.'_asset_'.$assetid.'_attr_'.$attr_name.'_'.$offset;
					?>
					<tr>
						<td><?php echo $attr_name; ?></th>
						<td><label for="<?php echo $html_id; ?>"><?php $this->_printHighlightedMatch($this->_running_vars['search_string'], $attr_details['value'], $offset); ?></label></td>
						<td class="match-checkbox"><?php check_box($html_id, 1, FALSE, 'updateParentCheckboxes(this)'); ?></td>
					</tr>
					<?php
				}
			}
			?>
			</tbody>
		</table>
		<?php

	}//end _printAttributeMatches()


	/**
	* Print matches in an asset's metadata
	*
	* @param string	$assetid		The assets id
	* @param string	$prefix			Prefix for the fields
	* @param array	&$meta_matches	Matches
	*
	* @return void
	* @access private
	*/
	function _printMetadataMatches($assetid, $prefix, &$meta_matches)
	{
		?>
		<table class="sq-backend-table expandable component" border="1">
			<thead>
				<tr>
					<th colspan="3" onclick="toggleTBodyDisplay(this)">
						<?php echo translate('asset_metadata'); ?>
					</th>
					<th class="match-checkbox" style="text-align: right">
						<?php check_box($prefix.'_asset_'.$assetid.'_attr_all', 1, FALSE, 'toggleTBodyCheckboxes(this); updateParentCheckboxes(this);', 'class="component-checkbox"'); ?>
					</th>
				</tr>
			</thead>
			<tbody style="display: none">
				<tr>
					<th><?php echo translate('field_id'); ?></th>
					<th><?php echo translate('match'); ?></th>
					<th>&nbsp;</th>
				</tr>
			<?php
			foreach ($meta_matches as $field_id => $field_details) {
				foreach ($field_details['matches'] as $offset) {
					$html_id = $prefix.'_asset_'.$assetid.'_metadata_'.$field_id.'_'.$offset;
					?>
					<tr>
						<td><?php echo get_asset_tag_line($field_id); ?></th>
						<td><?php echo $field_details['name']; ?></td>
						<td><label for="<?php echo $html_id; ?>"><?php $this->_printHighlightedMatch($this->_running_vars['search_string'], $field_details['value'], $offset); ?></label></td>
						<td class="match-checkbox"><?php check_box($html_id, 1, FALSE, 'updateParentCheckboxes(this)'); ?></td>
					</tr>
					<?php
				}
			}
			?>
			</tbody>
		</table>
		<?php

	}//end _printMetadataMatches()


	/**
	* Print matches in a metadata schema
	*
	* @param string	$assetid			The assets id
	* @param string	$prefix				Prefix for the fields
	* @param array	&$schema_matches	Matches
	*
	* @return void
	* @access private
	*/
	function _printSchemaMatches($assetid, $prefix, &$schema_matches)
	{
		?>
		<table class="sq-backend-table expandable component" border="1">
			<thead>
				<tr>
					<th colspan="2" onclick="toggleTBodyDisplay(this)">
						<?php echo translate('metadata_schema'); ?>
					</th>
					<th class="match-checkbox" style="text-align: right">
						<?php check_box($prefix.'_asset_'.$assetid.'_attr_all', 1, FALSE, 'toggleTBodyCheckboxes(this); updateParentCheckboxes(this);', 'class="component-checkbox"'); ?>
					</th>
				</tr>
			</thead>
			<tbody style="display: none">
				<tr>
					<th><?php echo translate('field_id'); ?></th>
					<th><?php echo translate('match'); ?></th>
					<th>&nbsp</th>
				</tr>
			<?php
			foreach ($schema_matches as $field_id => $schema_details) {
				foreach ($schema_details['matches'] as $offset) {
					$html_id = $prefix.'_asset_'.$assetid.'_schemafield_'.$field_id.'_'.$offset;
					?>
					<tr>
						<td><?php echo get_asset_tag_line($field_id); ?></th>
						<td><label for="<?php echo $html_id; ?>"><?php $this->_printHighlightedMatch($this->_running_vars['search_string'], $schema_details['value'], $offset); ?></label></td>
						<td class="match-checkbox"><?php check_box($html_id, 1, FALSE, 'updateParentCheckboxes(this)'); ?></td>
					</tr>
					<?php
				}
			}
			?>
			</tbody>
		</table>
		<?php

	}//end _printSchemaMatches()


	/**
	* Print matches from an asset's content
	*
	* @param string	$assetid		The assets id
	* @param string	$prefix			Prefix for the fields
	* @param array	&$div_matches	Matches
	*
	* @return void
	* @access private
	*/
	function _printContentMatches($assetid, $prefix, &$div_matches)
	{
		?>
		<table class="sq-backend-table expandable component" border="1">
			<thead>
				<tr>
					<th colspan="2" onclick="toggleTBodyDisplay(this)">
						<?php echo translate('asset_contents'); ?>
					</th>
					<th class="match-checkbox" style="text-align: right">
						<?php check_box($prefix.'_asset_'.$assetid.'_content_all', 1, FALSE, 'toggleTBodyCheckboxes(this); updateParentCheckboxes(this);', 'class="component-checkbox"'); ?>
					</th>
				</tr>
			</thead>
			<tbody style="display: none">
				<tr>
					<th><?php echo translate('div_id'); ?></th>
					<th><?php echo translate('match'); ?></th>
					<th>&nbsp;</th>
				</tr>
			<?php
			foreach ($div_matches as $div_id => $div_details) {
				foreach ($div_details['matches'] as $offset) {
					$html_id = $prefix.'_asset_'.$assetid.'_content_'.$div_id.'_'.$offset;
					?>
					<tr>
						<td><?php echo $div_id; ?></th>
						<td><label for="<?php echo $html_id; ?>"><?php $this->_printHighlightedMatch($this->_running_vars['search_string'], $div_details['value'], $offset); ?></label></td>
						<td class="match-checkbox"><?php check_box($html_id, 1, FALSE, 'updateParentCheckboxes(this)'); ?></td>
					</tr>
					<?php
				}
			}
			?>
			</tbody>
		</table>
		<?php

	}//end _printContentMatches()


	/**
	* Process replacements in asset contents
	*
	* @param array	&$step_data	a reference to the step data for this step
	* @param string	$prefix		a string for prefixing form variables
	*
	* @return boolean
	* @access public
	*/
	function processContent(&$step_data, $prefix)
	{
		$GLOBALS['SQ_SYSTEM']->changeContext($this->_running_vars['contextid']);

		$am =& $GLOBALS['SQ_SYSTEM']->am;
		$search_string = $this->_running_vars['search_string'];
		$replace_string = $this->_running_vars['replace_string'];
		$assetid = key($this->_running_vars['content_replacements']);
		$match_case = $this->_running_vars['match_case'];
		$asset = $am->getAssetInfo(Array($assetid));

		$step_data['message'] = translate('sch_tool_global_sr_replacing_with_in', $search_string, $replace_string, $asset[$assetid]['name']);

		$asset_contents = $this->_running_vars['search_data'][$assetid]['contents'];
		foreach ($asset_contents as $contents_id => $contents_value) {
			$new_data = $this->_replaceOccurrences($search_string, $replace_string, $contents_value['value'], $contents_value['matches'], $match_case);
			$am->setEditableContents($contents_id, $new_data);
		}

		$GLOBALS['SQ_SYSTEM']->am->forgetAsset($asset_contents);

		unset($this->_running_vars['content_replacements'][$assetid]);

		$this->_running_vars['done_assetids'][] = $assetid;

		if (empty($this->_running_vars['content_replacements'])) {
			unset($this->_running_vars['done_assetids']);
			$step_data['percent_done'] = 100;
			$step_data['complete']     = TRUE;
		} else {
			$step_data['percent_done'] = (count($this->_running_vars['done_assetids']) / $this->_running_vars['content_replacements_total']) * 100;
			$step_data['complete']     = FALSE;
		}

		$GLOBALS['SQ_SYSTEM']->restoreContext();

		return TRUE;

	}//end processContent()


	/**
	* Process replacements in schema
	*
	* @param array	&$step_data	a reference to the step data for this step
	* @param string	$prefix		a string for prefixing form variables
	*
	* @return boolean
	* @access public
	*/
	function processSchemas(&$step_data, $prefix)
	{
		$GLOBALS['SQ_SYSTEM']->changeContext($this->_running_vars['contextid']);

		$am =& $GLOBALS['SQ_SYSTEM']->am;

		$search_string = $this->_running_vars['search_string'];
		$replace_string = $this->_running_vars['replace_string'];
		$assetid = key($this->_running_vars['schema_replacements']);
		$match_case = $this->_running_vars['match_case'];
		$asset = $am->getAssetInfo(Array($assetid));

		$step_data['message'] = translate('sch_tool_global_sr_replacing_with_in', $search_string, $replace_string, $asset[$assetid]['name']);

		foreach ($this->_running_vars['schema_replacements'][$assetid] as $fieldid => $data) {
			$new_data = $this->_replaceOccurrences($search_string, $replace_string, $data['value'], $data['matches'], $match_case);
			$field = $am->getAsset($fieldid);
			$value_component = Array();
			$field->decodeValueString($new_data, $default_data, $value_component);
			$field->setAttrValue('default', $default_data);
			$field->saveAttributes();
			$am->forgetAsset($field);
		}

		unset($this->_running_vars['schema_replacements'][$assetid]);

		$this->_running_vars['done_assetids'][] = $assetid;

		if (empty($this->_running_vars['schema_replacements'])) {
			unset($this->_running_vars['done_assetids']);
			$step_data['percent_done'] = 100;
			$step_data['complete']     = TRUE;
		} else {
			$step_data['percent_done'] = (count($this->_running_vars['done_assetids']) / $this->_running_vars['schema_replacements_total']) * 100;
			$step_data['complete']     = FALSE;
		}

		$GLOBALS['SQ_SYSTEM']->restoreContext();

		return TRUE;

	}//end processSchemas()


	/**
	* Proccess replacements in Attribute fields
	*
	* @param array	&$step_data	a reference to the step data for this step
	* @param string	$prefix		a string for prefixing form variables
	*
	* @return boolean
	* @access public
	*/
	function processAttributes(&$step_data, $prefix)
	{
		$GLOBALS['SQ_SYSTEM']->changeContext($this->_running_vars['contextid']);

		$am =& $GLOBALS['SQ_SYSTEM']->am;

		$search_string = $this->_running_vars['search_string'];
		$replace_string = $this->_running_vars['replace_string'];
		$match_case = $this->_running_vars['match_case'];
		$assetid = key($this->_running_vars['attribute_replacements']);

		$asset = $am->getAsset($assetid);

		$step_data['message'] = translate('sch_tool_global_sr_replacing_with_in', $search_string, $replace_string, $asset->name);

		foreach ($this->_running_vars['attribute_replacements'][$assetid] as $attrid => $data) {
			$new_data = $this->_replaceOccurrences($search_string, $replace_string, $data['value'], $data['matches'], $match_case);
			$asset->setAttrValue($attrid, $new_data);
		}
		$asset->saveAttributes();

		$am->forgetAsset($asset);
		unset($this->_running_vars['attribute_replacements'][$assetid]);

		$this->_running_vars['done_assetids'][] = $assetid;

		if (empty($this->_running_vars['attribute_replacements'])) {
			unset($this->_running_vars['done_assetids']);
			$step_data['percent_done'] = 100;
			$step_data['complete']     = TRUE;
		} else {
			$step_data['percent_done'] = (count($this->_running_vars['done_assetids']) / $this->_running_vars['attribute_replacements_total']) * 100;
			$step_data['complete']     = FALSE;
		}

		$GLOBALS['SQ_SYSTEM']->restoreContext();

		return TRUE;

	}//end processAttributes()


	/**
	* Process replacements in Metadata fields
	*
	* @param array	&$step_data	a reference to the step data for this step
	* @param string	$prefix		a string for prefixing form variables
	*
	* @return boolean
	* @access public
	*/
	function processMetadata(&$step_data, $prefix)
	{
		$GLOBALS['SQ_SYSTEM']->changeContext($this->_running_vars['contextid']);

		$am =& $GLOBALS['SQ_SYSTEM']->am;
		$mm = $GLOBALS['SQ_SYSTEM']->getMetadataManager();

		$search_string = $this->_running_vars['search_string'];
		$replace_string = $this->_running_vars['replace_string'];
		$match_case = $this->_running_vars['match_case'];
		$assetid = key($this->_running_vars['metadata_replacements']);

		$asset = $am->getAssetInfo(Array($assetid));

		$step_data['message'] = translate('sch_tool_global_sr_replacing_with_in', $search_string, $replace_string, $asset[$assetid]['name']);

		$metadata = $mm->getMetadata($assetid);
		// All context metadata files need to be regenerated if there is a non-contextable field changed
		$regen_all_contexts = FALSE;
		foreach ($this->_running_vars['metadata_replacements'][$assetid] as $fieldid => $data) {
			$new_data = $this->_replaceOccurrences($search_string, $replace_string, $data['value'], $data['matches'], $match_case);
			$metadata[$fieldid][0]['value'] = $new_data;
			$field = $GLOBALS['SQ_SYSTEM']->am->getAsset($fieldid);
			if (!$regen_all_contexts && (!$field->attr('is_contextable') || $field instanceof Metadata_Field_Select)) {
				$regen_all_contexts = TRUE;
			}
		}

		$mm->setMetadata($assetid, $metadata);
		if ($regen_all_contexts) {
			$all_contexts = $GLOBALS['SQ_SYSTEM']->getAllContexts();
			foreach ($all_contexts as $contextid => $context_data) {
				$mm->generateContentFile($assetid, FALSE, $contextid);
			}
		} else {
			$mm->generateContentFile($assetid);
		}

		unset($this->_running_vars['metadata_replacements'][$assetid]);

		$this->_running_vars['done_assetids'][] = $assetid;

		if (empty($this->_running_vars['metadata_replacements'])) {
			unset($this->_running_vars['done_assetids']);
			$step_data['percent_done'] = 100;
			$step_data['complete']     = TRUE;
		} else {
			$step_data['percent_done'] = (count($this->_running_vars['done_assetids']) / $this->_running_vars['metadata_replacements_total']) * 100;
			$step_data['complete']     = FALSE;
		}

		$GLOBALS['SQ_SYSTEM']->restoreContext();

		return TRUE;

	}//end processMetadata()


	/**
	* Get the indices at which needle occurs in haystack
	*
	* @param string	$needle		The string being looked for
	* @param string	$haystack	The string being looked through
	*
	* @return array
	* @access public
	*/
	function _getOccurrences($needle, $haystack)
	{
		$occurrences = Array();
		$offset = 0;
		$needle = strtolower($needle);
		$haystack = strtolower($haystack);
		$result = strpos($haystack, $needle);
		while (FALSE !== $result) {
			$occurrences[] = $result;
			$offset = $result + strlen($needle);
			$result = strpos($haystack, $needle, $offset);
		}

		// Bug Fix 2076 and additional fix for Bug Fix 1575
		// Only perform the following block if html encoded needle is not the same as
		// the original needle. Or not we have the duplicated entries in the result array
		$html_encoded_needle = htmlentities($needle);
		if ($needle != $html_encoded_needle) {
			$offset = 0;
			$result = strpos($haystack, $html_encoded_needle);
			while (FALSE !== $result) {
				$occurrences[] = $result;
				$offset = $result + strlen($needle);
				$result = strpos($haystack, $html_encoded_needle, $offset);
			}
		}
		return $occurrences;

	}//end _getOccurrences()


	/**
	* Replace all instances of needle in the haystack that occur in offsets
	*
	* @param string		$needle		The string being looked for
	* @param string		$new_needle	The string to insert
	* @param string		$haystack	The string being looked through
	* @param array		$offsets	Indices of instances you wish to replace
	* @param boolean	$case_match	Whether to match capitilisation of the needle in replace
	*
	* @return string
	* @access public
	*/
	function _replaceOccurrences($needle, $new_needle, $haystack, $offsets, $case_match=FALSE)
	{
		if (empty($offsets)) return $haystack;
		$search_len = strlen($needle);
		$encoded_search_len = strlen(htmlentities($needle));
		$result_str = '';
		$last_offset = 0;
		foreach ($offsets as $id => $offset) {
			$encoded = (strtolower(substr($haystack, $offset, $search_len)) != strtolower($needle));
			$this_search_len = $encoded ? $encoded_search_len : $search_len;
			$rep_str = $new_needle;
			if ($case_match) {
				$rep_str = $this->_matchCapitals(substr($haystack, $offset, $this_search_len), $new_needle);
			}
			$result_str .= substr($haystack, $last_offset, $offset - $last_offset).$rep_str;
			$last_offset = $offset + $this_search_len;
		}
		$result_str .=  substr($haystack, $last_offset);
		return $result_str;

	}//end _replaceOccurrences()


	/**
	* Match capitalisation of first string in the second
	*
	* @param string	$teacher	The string to match
	* @param string	$student	The string to capitalise
	*
	* @return string
	* @access public
	*/
	function _matchCapitals($teacher, $student)
	{
		$ret_str = $student;
		if (strtoupper($teacher) == $teacher) {
			$ret_str = strtoupper($student);
		} else if (strtoupper($teacher[0]) == $teacher[0]) {
			$ret_str[0] = strtoupper($ret_str[0]);
		} else if (strtolower($teacher[0]) == $teacher[0]) {
			$ret_str[0] = strtolower($ret_str[0]);
		}
		return $ret_str;

	}//end _matchCapitals()


	/**
	* Fill a table with highlighted search terms.
	*
	* @param string	$needle		The string to match
	* @param string	$haystack	The string to look in
	*
	* @return void
	* @access public
	*/
	function _printRows($needle, $haystack)
	{
		$occ = $this->_getOccurrences($needle, $haystack);
		foreach ($occ as $id => $offset) {
			echo '<tr><td><input type="checkbox" /></td><td>';
			$this->_printHighlightedMatch($needle, $haystack, $offset);
			echo '</td></tr>';
		}

	}//end _printRows()


	/**
	* Prints an example of $needle, in $haystack with highlighting
	*
	* @param string	$needle		The string to highlight for
	* @param string	$haystack	The string to look in
	* @param int	$offset		Where to find $needle
	*
	* @return void
	* @access public
	*/
	function _printHighlightedMatch($needle, $haystack, $offset)
	{
		if (substr($haystack, $offset, strlen($needle)) != $needle) {
			$needle = htmlentities($needle);
		}
		$back_limit = $this->_findBackBreak($haystack, $offset);
		$forward_limit = $this->_findForwardbreak($haystack, $offset + strlen($needle));
		echo htmlentities(ltrim(substr($haystack, $back_limit, $offset - $back_limit), '\n\r'));
		echo '<span style="background:yellow">'.$needle.'</span>';
		echo htmlentities(rtrim(substr($haystack, $offset + strlen($needle), $forward_limit - ($offset + strlen($needle)))));

	}//end _printHighlightedMatch()


	/**
	* Find a point in the string behind the current point to cutoff display at
	*
	* @param string	$haystack	The string to check
	* @param int	$offset		Where to start
	* @param int	$limit		maximum number of characters
	*
	* @return string
	* @access public
	*/
	function _findBackBreak($haystack, $offset, $limit=20)
	{
		$ret = $offset;
		while ($ret > $offset - 200 && $ret > 0) {
			if ($haystack[$ret] == '>') {
				break;
			} else if ($haystack[$ret] == '<') {
				return $ret;
			}
			$ret--;
		}
		if ($ret == 0) return $ret;
		$ret = $offset;
		$oldspace = $ret - $limit;
		while ($ret > $offset - $limit && $ret > 0) {
			switch ($haystack[$ret]) {
				case '\n':
				case '<':
						return $ret ;
				case ' ':
				case '.':
				case ';':
				case ')':
					$oldspace = $ret;
				break;

			}
			$ret--;
		}
		return $oldspace;

	}//end _findBackBreak()


	/**
	* Find a point in the string to cutoff display at
	*
	* @param string	$haystack	The string to check
	* @param int	$offset		Where to start
	* @param int	$limit		maximum number of characters
	*
	* @return string
	* @access public
	*/
	function _findForwardBreak($haystack, $offset, $limit=55)
	{
		$end_tag = $offset;
		while (($end_tag < $offset + 200) && ($end_tag < strlen($haystack))) {
			if ($haystack[$end_tag] == '<') {
				break;
			} else if ($haystack[$end_tag] == '>') {
				$end_tag++;
				break;
			}
			$end_tag++;
		}
		$ret = $offset;
		$oldspace = $offset + $limit;
		while (($ret < $offset + $limit) && ($ret < strlen($haystack))) {
			switch ($haystack[$ret]) {
				case '\n':
				case '<br>':
					return $ret + 1;
				break;
				case ' ':
				case '.':
				case ';':
				case ')':
				case '-':
				case ',':
				case '!':
					$oldspace = $ret + 1;
				break;
			}
			$ret++;
		}
		if ($end_tag > $oldspace) return $end_tag;
		return $oldspace;

	}//end _findForwardBreak()


	/**
	* Insert search matches into _running_vars
	*
	* @return void
	* @access public
	*/
	function _setupMatches()
	{
		foreach ($this->_running_vars['search_data'] as $assetid => $details) {
			if (isset($details['attributes'])) {
				foreach ($details['attributes'] as $attr_name => $attr_val) {
					$this->_running_vars['search_data'][$assetid]['attributes'][$attr_name] = Array('value' => $attr_val);
					$this->_running_vars['search_data'][$assetid]['attributes'][$attr_name]['matches'] = $this->_getOccurrences($this->_running_vars['search_string'], $attr_val);
				}
			}
			if (isset($details['contents'])) {
				foreach ($details['contents'] as $div_id => $div_cont) {
					$this->_running_vars['search_data'][$assetid]['contents'][$div_id] = Array('value' => $div_cont);
					$this->_running_vars['search_data'][$assetid]['contents'][$div_id]['matches'] = $this->_getOccurrences($this->_running_vars['search_string'], $div_cont);
				}
			}
			if (isset($details['metadata'])) {
				foreach ($details['metadata'] as $schema_id => $schema_details) {
					$this->_running_vars['search_data'][$assetid]['metadata'][$schema_id]['matches'] = $this->_getOccurrences($this->_running_vars['search_string'], $schema_details['value']);
				}
			}
			if (isset($details['schema'])) {
				foreach ($details['schema'] as $schema_id => $schema_details) {
					$this->_running_vars['search_data'][$assetid]['schema'][$schema_id]['matches'] = $this->_getOccurrences($this->_running_vars['search_string'], $schema_details['value']);
				}
			}
		}

	}//end _setupMatches()


	/**
	* Unset any search matches that are unneeded
	*
	* @param string	$prefix	Prefix for the fields
	*
	* @return void
	* @access public
	*/
	function _updateSearchData($prefix)
	{
		foreach ($this->_running_vars['search_data'] as $assetid => $values) {
			// attributes
			if (!isset($_REQUEST[$prefix.'_asset_'.$assetid.'_all'])) {
				if (isset($this->_running_vars['search_data'][$assetid]['attributes'])) {
					if (!isset($_REQUEST[$prefix.'_asset_'.$assetid.'_attr_all'])) {
						foreach ($this->_running_vars['search_data'][$assetid]['attributes'] as $attr_name => $attr_details) {
							foreach ($attr_details['matches'] as $m) {
								if (!isset($_REQUEST[$prefix.'_asset_'.$assetid.'_attr_'.$attr_name.'_'.$m])) {
									$i = array_search($m, $this->_running_vars['search_data'][$assetid]['attributes'][$attr_name]['matches']);
									unset($this->_running_vars['search_data'][$assetid]['attributes'][$attr_name]['matches'][$i]);
								}
							}
							if (empty($this->_running_vars['search_data'][$assetid]['attributes'][$attr_name]['matches'])) {
								unset($this->_running_vars['search_data'][$assetid]['attributes'][$attr_name]);
							}
						}
						if (empty($this->_running_vars['search_data'][$assetid]['attributes'])) {
							unset($this->_running_vars['search_data'][$assetid]['attributes']);
						}
					}
				}
				// contents
				if (isset($this->_running_vars['search_data'][$assetid]['contents'])) {
					if (!isset($_REQUEST[$prefix.'_asset_'.$assetid.'_content_all'])) {
						foreach ($this->_running_vars['search_data'][$assetid]['contents'] as $div_id => $div_details) {
							foreach ($div_details['matches'] as $m) {
								if (!isset($_REQUEST[$prefix.'_asset_'.$assetid.'_content_'.$div_id.'_'.$m])) {
									$i = array_search($m, $this->_running_vars['search_data'][$assetid]['contents'][$div_id]['matches']);
									unset($this->_running_vars['search_data'][$assetid]['contents'][$div_id]['matches'][$i]);
								}
							}
							if (empty($this->_running_vars['search_data'][$assetid]['contents'][$div_id]['matches'])) {
								unset($this->_running_vars['search_data'][$assetid]['contents'][$div_id]);
							}
						}
						if (empty($this->_running_vars['search_data'][$assetid]['contents'])) {
							unset($this->_running_vars['search_data'][$assetid]['contents']);
						}
					}
				}
				// metadata
				if (isset($this->_running_vars['search_data'][$assetid]['metadata'])) {
					if (!isset($_REQUEST[$prefix.'_asset_'.$assetid.'_metadata_all'])) {
						foreach ($this->_running_vars['search_data'][$assetid]['metadata'] as $field_id => $field_details) {
							foreach ($field_details['matches'] as $m) {
								if (!isset($_REQUEST[$prefix.'_asset_'.$assetid.'_metadata_'.$field_id.'_'.$m])) {
									$i = array_search($m, $this->_running_vars['search_data'][$assetid]['metadata'][$field_id]['matches']);
									unset($this->_running_vars['search_data'][$assetid]['metadata'][$field_id]['matches'][$i]);
								}
							}
							if (empty($this->_running_vars['search_data'][$assetid]['metadata'][$field_id]['matches'])) {
								unset($this->_running_vars['search_data'][$assetid]['metadata'][$field_id]);
							}
						}
						if (empty($this->_running_vars['search_data'][$assetid]['metadata'])) {
							unset($this->_running_vars['search_data'][$assetid]['metadata']);
						}
					}
				}
				// schema
				if (isset($this->_running_vars['search_data'][$assetid]['schema'])) {
					if (!isset($_REQUEST[$prefix.'_asset_'.$assetid.'_schema_all'])) {
						foreach ($this->_running_vars['search_data'][$assetid]['schema'] as $field_id => $field_details) {
							foreach ($field_details['matches'] as $m) {
								if (!isset($_REQUEST[$prefix.'_asset_'.$assetid.'_schemafield_'.$field_id.'_'.$m])) {
									$i = array_search($m, $this->_running_vars['search_data'][$assetid]['schema'][$field_id]['matches']);
									unset($this->_running_vars['search_data'][$assetid]['schema'][$field_id]['matches'][$i]);
								}
							}
							if (empty($this->_running_vars['search_data'][$assetid]['schema'][$field_id]['matches'])) {
								unset($this->_running_vars['search_data'][$assetid]['schema'][$field_id]);
							}
						}
						if (empty($this->_running_vars['search_data'][$assetid]['schema'])) {
							unset($this->_running_vars['search_data'][$assetid]['schema']);
						}
					}
				}
			}//end if
			if (empty($this->_running_vars['search_data'][$assetid])) {
				unset($this->_running_vars['search_data'][$assetid]);
			}
		}//end foreach

	}//end _updateSearchData()


}//end class


?>
