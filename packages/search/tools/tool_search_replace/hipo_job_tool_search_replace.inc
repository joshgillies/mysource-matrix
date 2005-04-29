<?php
/**
* +--------------------------------------------------------------------+
* | Squiz.net Commercial Module Licence                                |
* +--------------------------------------------------------------------+
* | Copyright (c) Squiz Pty Ltd (ACN 084 670 600).                     |
* +--------------------------------------------------------------------+
* | This source file is not open source or freely usable and may be    |
* | used subject to, and only in accordance with, the Squiz Commercial |
* | Module Licence.                                                    |
* | Please refer to http://www.squiz.net/licence for more information. |
* +--------------------------------------------------------------------+
*
* $Id: hipo_job_tool_search_replace.inc,v 1.7 2005/04/29 05:38:41 gsherwood Exp $
*
*/

require_once SQ_SYSTEM_ROOT.'/core/hipo/hipo_job.inc';

/**
* Highly Intensive Processing Object (HIPO) to perform search and replace on asset attributes and metadata
*
* @author  Nathan de Vries <ndvries@squiz.net>
* @version $Revision: 1.7 $
* @package MySource_Matrix_Packages
* @subpackage search
*/
class HIPO_Job_Tool_Search_Replace extends HIPO_Job
{

	/**
	* The name of this HIPO job (should not be modified by code)
	* @var string
	*/
	var $hipo_name = 'Global Search and Replace';

	/**
	* Array of steps to perform in this HIPO
	* @var Array
	*/
	var $_steps = Array(
						Array(
							  'name'          => 'Confirmation',
							  'function_call' => Array(
												 	  'paint_function'   => 'paintConfirmation',
													   'process_function' => 'processConfirmation',
													   ),
							  'running_mode'  => 'web',
							  'skip_step'     => false,
							  'auto_step'     => false,
							  'allow_cancel'  => true,
							  'percent_done'  => 0,
							  'complete'      => false,
							  'message'       => '',
							  ),
						Array(
							  'name'         => 'Acquiring Locks',
							  'hipo_job'     => Array(
													  'init_details_function' => 'getLockAssetDetails',
													  ),
							  'running_mode' => 'server',
							  'auto_step'    => true,
							  'allow_cancel' => false,
							  'percent_done' => 0,
							  'complete'     => false,
							  'message'      => '',
							  ),
						Array(
							  'name'          => 'Replacing Asset Attributes',
							  'function_call' => Array(
													   'process_function' => 'processAttributes',
													   ),
							  'running_mode'  => 'server',
							  'skip_step'     => false,
							  'auto_step'     => true,
							  'allow_cancel'  => true,
							  'percent_done'  => 0,
							  'complete'      => false,
							  'message'       => '',
							  ),
						Array(
							  'name'          => 'Replacing Asset Contents',
							  'function_call' => Array(
													   'process_function' => 'processContent',
													   ),
							  'running_mode'  => 'server',
							  'skip_step'     => false,
							  'auto_step'     => true,
							  'allow_cancel'  => true,
							  'percent_done'  => 0,
							  'complete'      => false,
							  'message'       => '',
							  ),
						Array(
							  'name'          => 'Replacing Asset Metadata',
							  'function_call' => Array(
													   'process_function' => 'processMetadata',
													   ),
							  'running_mode'  => 'server',
							  'skip_step'     => false,
							  'auto_step'     => true,
							  'allow_cancel'  => true,
							  'percent_done'  => 0,
							  'complete'      => false,
							  'message'       => '',
							  ),
						Array(
							  'name'          => 'Replacing Schema Values',
							  'function_call' => Array(
													   'process_function' => 'processSchemas',
													   ),
							  'running_mode'  => 'server',
							  'skip_step'     => false,
							  'auto_step'     => true,
							  'allow_cancel'  => true,
							  'percent_done'  => 0,
							  'complete'      => false,
							  'message'       => '',
							  ),
						Array(
							  'name'          => 'Releasing Locks',
							  'function_call' => Array(
							  						 'process_function' => 'releaseLocks',
													   ),
							  'running_mode'  => 'server',
							  'auto_step'     => true,
							  'skip_step'     => false,
							  'allow_cancel'  => false,
							  'percent_done'  => 0,
							  'complete'      => false,
							  'message'       => '',
							  ),
						Array(
							  'name'         => 'Re-Generating Metadata',
							  'hipo_job'     => Array(
													  'init_details_function' => 'getRegenerateMetadataDetails',
													  ),
							  'running_mode' => 'server',
							  'auto_step'    => true,
							  'allow_cancel' => false,
							  'percent_done' => 0,
							  'complete'     => false,
							  'message'      => '',
							  ),
						Array(
							  'name'         => 'Re-Indexing Assets',
							  'hipo_job'     => Array(
													  'init_details_function' => 'getReindexAssetDetails',
													  ),
							  'running_mode' => 'server',
							  'auto_step'    => true,
							  'allow_cancel' => false,
							  'percent_done' => 0,
							  'complete'     => false,
							  'message'      => '',
							  ),
						);


	/**
	* Constructor
	*
	* @param string	$code_name	a unique codename the HIPO
	*/
	function HIPO_Job_Tool_Search_Replace($code_name='')
	{
		$this->_hipo_vars['job_dir'] = SQ_PACKAGES_PATH.'/packages/search/tools/tool_search_replace';
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
	* Prepares the running vars and gets the children of the asset in question
	*
	* @return boolean
	* @access public
	*/
	function prepare()
	{
		return parent::prepare();

	}//end prepare()


	/**
	* This function is used to set-up the details for locking the assets by the nested HIPO_Job
	*
	* @param string		&$job_type
	* @param array		&$running_vars
	* @param array		&$options
	*
	* @return void
	* @access public
	*/
	function getLockAssetDetails(&$job_type, &$running_vars, &$options)
	{
		$job_type = 'hipo_job_acquire_locks';

		$options['auto_complete'] = true;

		$running_vars = Array(
							  'assetids'          => array_keys($this->_running_vars['search_data']),
							  'lock_type'         => Array('all'),
							  'forceably_acquire' => false,
							  'dependants_only'   => true,
							  );

	}//end getLockAssetDetails()


	/**
	* This function is used to set-up the details for the regenerate metadata nested HIPO_Job
	*
	* @param string		&$job_type
	* @param array		&$running_vars
	* @param array		&$options
	*
	* @return void
	* @access public
	*/
	function getRegenerateMetadataDetails(&$job_type, &$running_vars, &$options)
	{
		$job_type = 'hipo_job_regenerate_metadata';

		$options['auto_complete'] = true;

		$running_vars = Array(
							  'schemaids' => $this->_running_vars['schemaids'],
							  );

	}//end getRegenerateMetadataDetails()


	/**
	* This function is used to set-up the details for the reindex assets nested HIPO_Job
	*
	* @param string		&$job_type
	* @param array		&$running_vars
	* @param array		&$options
	*
	* @return void
	* @access public
	*/
	function getReindexAssetDetails(&$job_type, &$running_vars, &$options, &$hipo_vars)
	{
		$job_type = 'hipo_job_tool_reindex';

		$options['auto_complete'] = true;

		$running_vars = Array(
							  'reindex_assets' => array_keys($this->_running_vars['search_data']),
							  );

		$hipo_vars = Array(
						   'job_dir' => SQ_PACKAGES_PATH.'/search/tools/tool_reindex',
						   );

	}//end getReindexAssetDetails()


	/**
	* Release the locks that were acquired while cloning assets
	*
	* @param array()	&$step_data	a reference to the array of information about the current step
	* @param string		$prefix		prefix for form vars
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

		$children = &$GLOBALS['SQ_SYSTEM']->am->getDependantChildren($assetid);

		foreach ($children as $dep_child_id => $dep_child_type_code) {
			$GLOBALS['SQ_SYSTEM']->am->releaseLock($dep_child_id, 'all');
		}

		$asset_info = &$GLOBALS['SQ_SYSTEM']->am->getAssetInfo(Array($assetid));

		$step_data['message'] = 'Releasing Locks on '.$asset_info[$assetid]['name'].' and its dependant assets';

		$GLOBALS['SQ_SYSTEM']->am->releaseLock($assetid, 'all');

		$this->_running_vars['done_assetids'][] = $assetid;

		if (empty($this->_running_vars['re_lock_assets'])) {
			unset($this->_running_vars['done_assetids']);
			$step_data['percent_done'] = 100;
			$step_data['complete']     = true;
		} else {
			$step_data['percent_done'] = (count($this->_running_vars['done_assetids']) / count(array_keys($this->_running_vars['search_data']))) * 100;
			$step_data['complete']     = false;
		}

		return true;

	}//end releaseLocks()


	/**
	* Paints some option for the user to specify what replacements are to be made
	* as a result of the search
	*
	* @param &array							$step_data	a reference to the step data for this step
	* @param &object Hipo_Backend_Outputter	$o			the hipo backend outputter object
	* @param string							$prefix		a string for prefixing form variables
	*
	* @return boolean
	* @access public
	*/
	function paintConfirmation(&$step_data, &$o, $prefix)
	{

		//create search string count array
		$search_count = Array();
		$search_string = $this->_running_vars['search_string'];

		foreach ($this->_running_vars['search_data'] as $assetid => $contents) {
			$search_count[$assetid] = 0;
			if (isset($contents['attributes'])) {
				foreach ($contents['attributes'] as $index => $value) {
					preg_match_all("/$search_string/i", $value, $matches);
					$search_count[$assetid] += count($matches[0]);
					unset($matches);
				}
			}
			if (isset($contents['metadata'])) {
				foreach ($contents['metadata'] as $index => $value) {
					preg_match_all("/$search_string/i", $value['value'], $matches);
					$search_count[$assetid] += count($matches[0]);
					unset($matches);
				}
			}
			if (isset($contents['contents'])) {
				foreach ($contents['contents'] as $index => $value) {
					preg_match_all("/$search_string/i", $value, $matches);
					$search_count[$assetid] += count($matches[0]);
					unset($matches);
				}
			}
			if (isset($contents['schema'])) {
				foreach ($contents['schema'] as $index => $value) {
					preg_match_all("/$search_string/i", $value['value'], $matches);
					$search_count[$assetid] += count($matches[0]);
					unset($matches);
				}
			}
		}

		$o->openSection('Search and Replace Confirmation');
		$o->openField('', 'replace_selection');

		?>
		<script type="text/javascript"><!--
		//<![CDATA[
			var checkState = 0;
			var expandState = 0;

			function setCheckboxes()
			{
				var tags = document.getElementsByTagName('input');
				for (var i = 0; i < tags.length; i++) {
					if(tags[i].type == 'checkbox')
						tags[i].checked = (checkState) ? 0 : 1;
				}
				checkState = (checkState) ? 0 : 1;
			}

			function setDisplayState()
			{
				var tags = document.getElementsByTagName('td');
				for (var i in tags) {
					if(tags[i].id && tags[i].id.indexOf('collapse[') == 0) {
						tags[i].style.display = (expandState) ? 'none': '';
					}
				}
				expandState = (expandState) ? 0 : 1;
			}
		//]]> -->
		</script>
		<style type="text/css">
			th.expander {
				width: 20px; 
				font-weight: bold; 
				cursor: pointer; 
				text-align: center
			}
			span.link {
				text-decoration: underline;
				cursor: pointer;
			}
		</style>
		<p style="text-align: center">
			<span class="link" id="select all" name="select all" onclick="setCheckboxes();">Select/Deselect All</span> &nbsp;&nbsp;&nbsp;|&nbsp;&nbsp;&nbsp;
			<span class="link" id="expand all" name="expand all" onclick="setDisplayState();" >Expand/Collapse All</span>
		</p>
		<?php

		foreach ($this->_running_vars['search_data'] as $assetid => $value) {
			$asset_info = &$GLOBALS['SQ_SYSTEM']->am->getAssetInfo(Array($assetid));
			?>
			<table class="sq-backend-table" style="margin-bottom: 10px">
			<tr>
				<th>
					<input type="checkbox" style="margin: 0 5px 0 0" name="<?php echo 'replace['.$assetid.']'; ?>"  /><?php echo $asset_info[$assetid]['name'].' [#'.$assetid.']    '; ?> ( <?php echo $search_count[$assetid]; echo ($search_count[$assetid] == 1) ? ' match': ' matches'; ?> )
				</th>
				<th class="expander" onclick="elt = document.getElementById('<?php echo 'collapse['.$assetid; ?>]'); elt.style.display = ((elt.style.display == 'none') ? '' : 'none');">+</th>
			</tr>
			<tr>
				<?php echo '<td id="collapse['.$assetid.']" style="display:none;" colspan="2">'; ?>
				<table width="100%">
				<?php

				if (!empty($value['attributes'])){
					?><tr><td><h3>Asset Attributes</h3><?php
					foreach ($value['attributes'] as $i => $v) {
							echo '<p><b>Name:</b> '.$i.'<br />';
							echo '<b>Value:</b> '.preg_replace("/($search_string)/i", '<span style="background: #ffff00;">\1</span>', $v).'</p>';
					}
					?></td></tr><?php
				}

				if (!empty($value['metadata'])) {
					?><tr><td><h3>Metadata</h3><?php
					foreach ($value['metadata'] as $metadataid => $metadata_contents) {
							echo '<p><b>Schema ID:</b> '.$metadataid.'<br />';
							echo '<b>Field Name:</b> '.$metadata_contents['name'].'<br />';
							echo '<b>Value:</b> '.preg_replace("/($search_string)/i", '<span style="background: #ffff00;">\1</span>', $metadata_contents['value']).'<br /></p>';

					}
					?></td></tr><?php
				}

				if (isset($value['contents'])) {
					?><tr><td><h3>Contents</h3><?php
					foreach ($value['contents'] as $content_id => $contents_value) {
							echo '<p><b>Content Div Id:</b> '.$content_id.'<br />';
							echo preg_replace("/($search_string)/i", '<span style="background: #ffff00;">\1</span>', $contents_value).'</p>';
					}

					?></td></tr><?php
				}

				if (isset($value['schema'])) {
					?><tr><td><h3>Schema</h3><?php
					foreach ($value['schema'] as $schemaid => $schema_contents) {
							?>
							<p><b>Schema ID:</b> <?php echo $schemaid; ?><br />
							<b>Field Name:</b> <?php echo $schema_contents['name']; ?><br />
							<?php
							echo '<b>Value:</b> '.preg_replace("/($search_string)/i", '<span style="background: #ffff00;">\1</span>', $schema_contents['value']).'<br /></p>';
					}
				}

				?>
					</table>
				</td>
			</tr>
			</table>
		<?php

		}
		?>
		<p style="text-align: center">
			<span class="link" id="select all" name="select all" onclick="setCheckboxes();">Select/Deselect All</span> &nbsp;&nbsp;&nbsp;|&nbsp;&nbsp;&nbsp;
			<span class="link" id="expand all" name="expand all" onclick="setDisplayState();" >Expand/Collapse All</span>
		</p>
		<?php

		$o->addHiddenField('form_submitted', 1);
		$o->closeField();
		$o->closeSection();

	}//end paintConfirmation()


	/**
	* Does nothing
	*
	* @param array		&$step_data	a reference to the step data for this step
	* @param String		$prefix		a string for prefixing form variables
	*
	* @return boolean
	* @access public
	*/
	function processConfirmation(&$step_data, $prefix)
	{

		if (isset($_REQUEST['form_submitted'])) {
			if (!isset($_REQUEST['replace'])) {
				$this->skipStep(1);
				$this->skipStep(2);
				$this->skipStep(3);
				$this->skipStep(4);
				$this->skipStep(5);
				$this->skipStep(6);
				$this->skipStep(7);
				$this->skipStep(8);
			}
			else {
				foreach ($this->_running_vars['search_data'] as $assetid => $values) {
					if (!isset($_REQUEST['replace'][$assetid])) {
						unset($this->_running_vars['search_data'][$assetid]);
					}
				}

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

				if ($this->_running_vars['attribute_replacements_total'] == 0) $this->skipStep(2);
				if ($this->_running_vars['content_replacements_total'] == 0) $this->skipStep(3);
				if ($this->_running_vars['metadata_replacements_total'] == 0) $this->skipStep(4);
				if ($this->_running_vars['schema_replacements_total'] == 0) {
					$this->skipStep(5);
					$this->skipStep(7);
				}
			}
			$step_data['percent_done'] = 100;
			$step_data['complete'] = true;
		}

		return true;

	}//end processConfirmation()


	/**
	* Proccess replacements in Attribute fields
	*
	* @param array		&$step_data	a reference to the step data for this step
	* @param String		$prefix		a string for prefixing form variables
	*
	* @return boolean
	* @access public
	*/
	function processAttributes(&$step_data, $prefix)
	{

		$am = &$GLOBALS['SQ_SYSTEM']->am;

		$search_string = $this->_running_vars['search_string'];
		$replace_string = $this->_running_vars['replace_string'];

		$assetid = key($this->_running_vars['attribute_replacements']);

		$asset = &$am->getAsset($assetid);

		$step_data['message'] = 'Replacing  "'.$search_string.'" with "'.$replace_string.'" in  "'.$asset->name.'"';

		foreach ($this->_running_vars['attribute_replacements'][$assetid] as $attrid => $data) {
			$new_data = preg_replace('/'.$search_string.'/i', $replace_string, $data);
			$asset->setAttrValue($attrid, $new_data);
		}
		$asset->saveAttributes();

		$am->forgetAsset($asset);
		unset($this->_running_vars['attribute_replacements'][$assetid]);

		$this->_running_vars['done_assetids'][] = $assetid;

		if (empty($this->_running_vars['attribute_replacements'])) {
			unset($this->_running_vars['done_assetids']);
			$step_data['percent_done'] = 100;
			$step_data['complete']     = true;
		} else {
			$step_data['percent_done'] = (count($this->_running_vars['done_assetids']) / $this->_running_vars['attribute_replacements_total']) * 100;
			$step_data['complete']     = false;
		}

		return true;

	}//end processAttributes()


	/**
	* Process replacements in Metadata fields
	*
	* @param array		&$step_data	a reference to the step data for this step
	* @param String		$prefix		a string for prefixing form variables
	*
	* @return boolean
	* @access public
	*/
	function processMetadata(&$step_data, $prefix)
	{
		$am = &$GLOBALS['SQ_SYSTEM']->am;
		$mm = &$GLOBALS['SQ_SYSTEM']->getMetadataManager();

		$search_string = $this->_running_vars['search_string'];
		$replace_string = $this->_running_vars['replace_string'];

		$assetid = key($this->_running_vars['metadata_replacements']);

		$asset = $am->getAssetInfo(Array($assetid));

		$step_data['message'] = 'Replacing  "'.$search_string.'" with "'.$replace_string.'" in  "'.$asset->name.'"';

		$metadata = $mm->getMetadata($assetid);
		foreach ($this->_running_vars['metadata_replacements'][$assetid] as $fieldid => $data) {
			$new_data = preg_replace('/'.$search_string.'/i', $replace_string, $data['value']);
			$metadata[$fieldid]['value'] = $new_data;
		}
		$mm->setMetadata($assetid, $metadata);
		$mm->regenerateMetadata($assetid);

		unset($this->_running_vars['metadata_replacements'][$assetid]);

		$this->_running_vars['done_assetids'][] = $assetid;

		if (empty($this->_running_vars['metadata_replacements'])) {
			unset($this->_running_vars['done_assetids']);
			$step_data['percent_done'] = 100;
			$step_data['complete']     = true;
		} else {
			$step_data['percent_done'] = (count($this->_running_vars['done_assetids']) / $this->_running_vars['metadata_replacements_total']) * 100;
			$step_data['complete']     = false;
		}

		return true;

	}//end processMetadata()


	/**
	* Process replacements in asset contents
	*
	* @param array		&$step_data	a reference to the step data for this step
	* @param String		$prefix		a string for prefixing form variables
	*
	* @return boolean
	* @access public
	*/
	function processContent(&$step_data, $prefix)
	{
		$am = &$GLOBALS['SQ_SYSTEM']->am;

		$search_string = $this->_running_vars['search_string'];
		$replace_string = $this->_running_vars['replace_string'];
		$assetid = key($this->_running_vars['content_replacements']);

		$asset = $am->getAssetInfo(Array($assetid));

		$step_data['message'] = 'Replacing  "'.$search_string.'" with "'.$replace_string.'" in  "'.$asset->name.'"';

		$asset_contents = $GLOBALS['SQ_SYSTEM']->am->getEditableContents($assetid);
		foreach ($asset_contents as $contents_id => $contents_value) {
			$new_data = preg_replace('/'.$search_string.'/i', $replace_string, $contents_value);
			$am->setEditableContents($contents_id, $new_data);
		}

		$GLOBALS['SQ_SYSTEM']->am->forgetAsset($asset_contents);

		unset($this->_running_vars['content_replacements'][$assetid]);

		$this->_running_vars['done_assetids'][] = $assetid;

		if (empty($this->_running_vars['content_replacements'])) {
			unset($this->_running_vars['done_assetids']);
			$step_data['percent_done'] = 100;
			$step_data['complete']     = true;
		} else {
			$step_data['percent_done'] = (count($this->_running_vars['done_assetids']) / $this->_running_vars['content_replacements_total']) * 100;
			$step_data['complete']     = false;
		}

		return true;

	}//end processContent()


	/**
	* Process replacements in schema
	*
	* @param array		&$step_data	a reference to the step data for this step
	* @param String		$prefix		a string for prefixing form variables
	*
	* @return boolean
	* @access public
	*/
	function processSchemas(&$step_data, $prefix)
	{
		$am = &$GLOBALS['SQ_SYSTEM']->am;

		$search_string = $this->_running_vars['search_string'];
		$replace_string = $this->_running_vars['replace_string'];

		$assetid = key($this->_running_vars['schema_replacements']);

		$asset = $am->getAssetInfo(Array($assetid));

		$step_data['message'] = 'Replacing  "'.$search_string.'" with "'.$replace_string.'" in  "'.$asset->name.'"';

		foreach ($this->_running_vars['schema_replacements'][$assetid] as $fieldid => $data) {
			$new_data = preg_replace('/'.$search_string.'/i', $replace_string, $data['value']);
			$field = &$am->getAsset($fieldid);
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
			$step_data['complete']     = true;
		} else {
			$step_data['percent_done'] = (count($this->_running_vars['done_assetids']) / $this->_running_vars['schema_replacements_total']) * 100;
			$step_data['complete']     = false;
		}

		return true;

	}//end processSchemas()


}//end class


?>
