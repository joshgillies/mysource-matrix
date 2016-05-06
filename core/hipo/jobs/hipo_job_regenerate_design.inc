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
* $Id: hipo_job_regenerate_design.inc,v 1.6 2012/08/30 01:09:19 ewang Exp $
*
*/

require_once SQ_SYSTEM_ROOT.'/core/hipo/hipo_job.inc';

/**
* Highly Intensive Processing Object (HIPO) to regenerate design parse file and propergate to customisations
*
* <pre>
* Initial Running Vars :
*    int           assetid          - the assetid of the design or design customisation
* </pre>
*
* @author  Edison Wang <ewang@squiz.com.au>
* @version $Revision: 1.6 $
* @package MySource_Matrix
* @subpackage HIPO
*/
class HIPO_Job_Regenerate_Design extends HIPO_Job
{


	/**
	* Constructor
	*
	* @param string	$code_name		a unique codename the HIPO
	*/
	function HIPO_Job_Regenerate_Design($code_name='')
	{
		$this->HIPO_Job($code_name);

	}//end constructor


	/**
	* Paints backend of this job
	*
	* @param object	&$o		reference to the backend outputter
	* @param string	$class	calling class name
	*
	* @return void
	* @access private
	*/
	public static function paintConfig(&$o, $class, $write_access)
	{
		// design regeneration threshhold HIPO config entry
		$o->openField(translate('Design Regenerate Threshold'));

		echo '<span class="sq-inline-fields-wrapper">';
		if ($write_access) {
			text_box($class.'[SQ_HIPO_REGENERATE_DESIGN_THRESHOLD]', SQ_HIPO_REGENERATE_DESIGN_THRESHOLD, 4);
		} else {
			echo SQ_HIPO_REGENERATE_DESIGN_THRESHOLD;
		}
		echo ' '.translate('assets');

		echo '</span>';

		$o->closeField();

	}//end paintConfig()


	/**
	* Sets some info about thresholds that this hipo requires.
	*
	* @return array
	* @access public
	*/
	public static function getConfigVars()
	{
		return Array(
				'SQ_HIPO_REGENERATE_DESIGN_THRESHOLD'	=> Array('editable'	=> 1, 'default'	=> 20),
			   );

	}//end getConfigVars()


	/**
	* Returns a unique codename the HIPO can use to ensure it is not being run twice
	*
	* The Acquire Lock HIPO appends the ID of the asset that the lock is being acquired on
	* (the top most parent) so that any other locking jobs on this asset do not start while it
	* is running. Due to locking of the interface, nobody should be allowed to run this HIPO at
	* the same time on the same parent asset.
	*
	* @return string
	* @access public
	*/
	function getCodeName()
	{
		$code_name = parent::getCodeName();
		return $code_name.'-'.md5($this->_running_vars['assetid']);

	}//end getCodeName()


	/**
	* Returns the (localised) HIPO name
	*
	* @return string
	* @access public
	*/
	function getHipoName()
	{
		return translate('Hipo Job Regenerate Design');

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
					'name'		=> translate('Hipo Job Regenerate Design'),
					'function_call'	=> Array(
							    'process_function'	=> 'processCustomisation',
							),
					'running_mode'	=> 'server',
					'auto_step'		=> true,
					'allow_cancel'	=> true,
					'percent_done'	=> 0,
					'complete'		=> false,
					'message'		=> '',
				),
			   );

	}//end getInitialStepData()





	/**
	* Performs the duties of this hipo, without showing any output on the frontend.
	*
	* @return boolean
	* @access public
	*/
	function freestyle()
	{
		while (!empty($this->_running_vars['todo_assetids'])) {
			if (!$this->processCustomisation($this->_steps[0], get_class($this))) {
				return false;
			}
		}
		return true;

	}//end freestyle()


	/**
	* Prepares the running vars and gets the children of the asset in question.
	*
	* @return boolean
	* @access public
	*/
	function prepare()
	{
		$parentid = isset($this->_running_vars['parentid']) ? $this->_running_vars['parentid'] : NULL;
		$assetid = isset($this->_running_vars['assetid']) ? $this->_running_vars['assetid'] : NULL;

		if (empty($assetid)) {
			trigger_error('Unable to run HIPO job; this job requires a design asset ID', E_USER_WARNING);
			return FALSE;
		}
		$this->_running_vars['todo_assetids'] = Array();
		$this->_running_vars['total_count'] = 0;
		$all_customisations = Array();
		$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
		$type = $asset->type();

		// first of first, generate parse file for itself
		if ($type === 'design_customisation' || $type === 'design_css_customisation') {
		    if(empty ($parentid)) {
			trigger_error('To regenerate a design customisation, parentid is required', E_USER_WARNING);
			return FALSE;
		    }
		    $parent = $GLOBALS['SQ_SYSTEM']->am->getAsset($parentid);
		    $asset->updateFromParent($parent, FALSE);
		}
		else {
		    $asset->generateDesignFile(FALSE);
		}


		 // get all children customisation to do
		$all_customisations = $this->_getCustomisations($assetid);
		$this->_running_vars['total_count'] += count($all_customisations);
		$this->_running_vars['todo_assetids'] = array_merge($this->_running_vars['todo_assetids'], $all_customisations);
		$this->_running_vars['done_assetids'] = Array();

		return parent::prepare();

	}//end prepare()


	/**
	* Proess a customisation
	*
	* @param array	&$step_data	a reference to the array of information about the current step
	* @param string	$prefix		prefix for form vars
	*
	* @return boolean
	* @access public
	*/
	function processCustomisation(&$step_data, $prefix)
	{
		if (!empty($this->_running_vars['todo_assetids'])) {
			$current_customisation = array_shift($this->_running_vars['todo_assetids']);

			$customisation = $GLOBALS['SQ_SYSTEM']->am->getAsset($current_customisation['child']);
			$design = $GLOBALS['SQ_SYSTEM']->am->getAsset($current_customisation['parent']);
			if ($acquired = $GLOBALS['SQ_SYSTEM']->am->acquireLock($customisation->id, 'all')) {

				// update individual customisation, but don't pass on
				$customisation->updateFromParent($design, FALSE);

				$this->_running_vars['done_assetids'][] =  $current_customisation;
				if ($acquired != 2) {
					$GLOBALS['SQ_SYSTEM']->am->releaseLock($customisation->id, 'all');
				}
			}


		}

		if (empty($this->_running_vars['todo_assetids'])) {
			$step_data['percent_done'] = 100;
			$step_data['complete']     = true;
		} else {
			$step_data['percent_done'] = (count($this->_running_vars['done_assetids']) / $this->_running_vars['total_count']) * 100;
			$step_data['complete']     = false;
		}

		return true;

	}//end processCustomisation()


	/**
	* Recursively retrieve all children customisation for a design/customisation
	*
	* @param int	$designid		design/customisation assetid
	*
	* @return array
	* @access private
	*/
	private function _getCustomisations ($designid) {
	    $customisation_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($designid, SQ_LINK_TYPE_2, 'design_customisation', TRUE, 'major', 'customisation');
	    $customisation_links = array_merge($customisation_links, $GLOBALS['SQ_SYSTEM']->am->getLinks($designid, SQ_LINK_TYPE_2, 'design_css_customisation', TRUE, 'major', 'customisation'));
	    if (empty($customisation_links)) return Array();

	    $children_customisations = Array();
	    foreach ($customisation_links as $link) {
		$children_customisations[] = Array('parent' => $designid, 'child' => $link['minorid']);
		$children_customisations = array_merge($children_customisations, $this->_getCustomisations($link['minorid']));
	    }
	    return $children_customisations;
	}
}//end class

?>
