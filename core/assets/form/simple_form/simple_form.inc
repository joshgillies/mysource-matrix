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
* $Id: simple_form.inc,v 1.44.8.3 2013/09/10 07:19:40 ewang Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset.inc';
require_once SQ_FUDGE_PATH.'/general/general.inc';
require_once SQ_CORE_PACKAGE_PATH.'/interfaces/bridge/bridge.inc';


/**
* Simple_Form
*
* Purpose
*
*
* @author  Luke Wright <lwright@squiz.net>
* @version $Revision: 1.44.8.3 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Simple_Form extends Asset implements Bridge
{


	/**
	* A list of submission errors
	*
	* @var Array(string)
	* @access private
	*/
	var $_errors = Array();


	/**
	* A list of submission answers
	*
	* @var Array(string)
	* @access private
	*/
	var $_answers = Array();


	/**
	* Constructor
	*
	* @param string	$assetid	the ID of the asset to load, if any
	*
	* @return void
	* @access public
	*/
	function Simple_Form($assetid=0)
	{
		$this->_ser_attrs = TRUE;
		$this->Asset($assetid);

	}//end constructor


//--        BRIDGE FUNCTIONS        --//


	/**
	* Returns a reference to the asset represented by the passed assetid or null on error
	*
	* @param int		$assetid		the asset id to be loaded
	* @param string		$type_code		if this exists then this object is used to
	*									load the asset, if not then the DB is queried
	*									to find out the asset type
	* @param boolean	$mute_errors	stops the outputting of errors in this fn, needed because
	*									you can't use the '@' operator when returning by reference
	*
	* @return object
	* @access public
	* @see Asset_Manager::&getAsset()
	*/
	function getAsset($assetid, $type_code='', $mute_errors=FALSE)
	{
		$asset = NULL;
		$id_parts = explode(':', $assetid);

		if (isset($id_parts[1])) {
			$shadowid = $id_parts[1];
		} else {
			return $asset;
		}

		// Questions follow the format '<assetid>:q<shadowid>'
		if ($shadowid{0} == 'q') {

			// pick up the question in question (!)
			$questions = $this->attr('questions');
			$questionid = substr($shadowid, 1, strlen($shadowid));
			if (empty($questions[$questionid])) return $asset;

			$q_type_code = $questions[$questionid]['type_code'];

			// not fussed about type code?
			if (empty($type_code)) $type_code = $q_type_code;

			// only give the asset back if of the right type code
			if (in_array($q_type_code, $GLOBALS['SQ_SYSTEM']->am->getTypeDescendants($type_code, TRUE))) {
				$questions[$questionid]['questionid'] = $questionid;
				$GLOBALS['SQ_SYSTEM']->am->includeAsset($q_type_code);

				// this eval returns the necessary object for the question type
				$asset = new $q_type_code($this->id, $questions[$questionid]);
			}
		}

		return $asset;

	}//end getAsset()


	/**
	* Return all links that this asset has to other assets
	*
	* @param int			$assetid			id of the the $side_of_link asset
	* @param int			$link_types			integer that can be the product of bitwise operations
	*											on the SQ_LINK_* constants
	* @param string|array	$type_code			the type of asset that is linked
	*											(eg 'Page', 'File', etc)
	*											if an array returns link if matches any
	*											of the array values
	* @param boolean		$strict_type_code	whether we are finding assets that are just a $type_code
	*											or $type_code and any of it's sub-classes
	* @param string			$side_of_link		Which side of the link this (the current) asset
	*											is on ('major' or 'minor')
	* @param string			$sort_by			a field in the asset table you would like the results
	*											sorted by (eg. name, short_name etc)
	* @param boolean		$dependant			Whether we want dependant links, non-dependent or (NULL) don't care
	* @param boolean		$exclusive			Whether we want exclusive links, non-exclusive or (NULL) don't care
	*
	* @return array
	* @access public
	*/
	function getLinks($assetid, $link_types, $type_code='', $strict_type_code=TRUE, $side_of_link='major', $sort_by=NULL, $dependant=NULL, $exclusive=NULL)
	{
		// there are no links away from the shadow asset (ie:questions)
		if (!is_numeric($assetid)) {
			$links = Array();
			if ($side_of_link == 'minor') {
				$return_link = FALSE;

				if ($type_code == '') {
					// not fussed what we're getting, so return me
					$return_link = TRUE;
				} else if ($strict_type_code) {
					// strict type code check
					$return_link = ($type_code == $this->type());
				} else {
					$return_link = (in_array($this->type(), $GLOBALS['SQ_SYSTEM']->am->getTypeDescendants($type_code, TRUE)));
				}

				if ($return_link) {
					$links[] = Array(
								'linkid'			=> 0,
								'majorid'			=> $this->id,
								'minorid'			=> $this->id.':'.$assetid,
								'major_type_code'	=> $this->type(),
								'value'				=> '',
								'link_type'			=> SQ_LINK_TYPE_2,
								'is_dependant'		=> TRUE,
								'is_exclusive'		=> FALSE,
								'sort_order'		=> 0,
								'locked'			=> 0,
							   );
				}
			}

			return $links;
		}//end if assetid is not numeric

		$links = Array();
		$new_sort_order = 0;

		// are we getting our bodycopy and/or nested forms (if they all exist)?
		$real_type_codes = Array(
							'bodycopy'		=> SQ_LINK_TYPE_2 | SQ_LINK_TYPE_3,
							'simple_form'	=> SQ_LINK_TYPE_2,
						   );

		foreach ($real_type_codes as $real_type_code => $allowed_link_types) {
			if ($type_code == '') {
				// not fussed what we're getting, so return me
				$get_type = TRUE;
			} else if ($strict_type_code) {
				$get_type = ($type_code == $real_type_code);
			} else {
				// specific type code set, non-strict check
				$get_type = (in_array($real_type_code, $GLOBALS['SQ_SYSTEM']->am->getTypeDescendants($type_code, TRUE)));
			}

			// we are getting this type?
			if ($get_type) {
				$links_query = $GLOBALS['SQ_SYSTEM']->am->generateGetLinksQuery($assetid, $link_types & $allowed_link_types, $real_type_code, FALSE);
				$type_links = NULL;
				try {
					$query = MatrixDAL::preparePdoQuery(implode(' ', $links_query['sql_array']));

					foreach($links_query['bind_vars'] as $bind_var => $bind_value) {
						MatrixDAL::bindValueToPdo($query, $bind_var, $bind_value);
					}
					$type_links = MatrixDal::executePdoAssoc($query);
				} catch (Exception $e) {
					throw new Exception($e->getMessage());
				}

				foreach (array_keys($type_links) as $key) {
					$type_links[$key]['sort_order'] = $new_sort_order;
					$links[] = $type_links[$key];
					$new_sort_order++;
				}
			}
		}

		// but we also need question shadow asset links
		if ($link_types & SQ_LINK_TYPE_2) {
			$questions = $this->attr('questions');

			// keep a cache of asset type codes if we are checking on a non-strict
			// type code, to stop us calling getTypeDescendants() all the time
			// (not needed if strict type check or if not fussed)
			$get_question_types = Array();

			foreach ($questions as $questionid => $data) {
				if ($type_code == '') {
					// not fussed what we're getting, so return me
					$get_question = TRUE;
				} else if ($strict_type_code) {
					// strict type code check
					$get_question = ($type_code == $data['type_code']);
				} else {
					// if we've already cached whether this question type is to be
					// returned, then we don't need to look it up again
					if (isset($get_question_types[$data['type_code']])) {
						$get_question = $get_question_types[$data['type_code']];
						$get_question_types[$data['type_code']] = $get_question;
					} else {
						$get_question = (in_array($data['type_code'], $GLOBALS['SQ_SYSTEM']->am->getTypeDescendants($type_code, TRUE)));
					}
				}

				if ($get_question) {
					$links[] = Array(
								'linkid'			=> 0,
								'majorid'			=> $this->id,
								'minorid'			=> $this->id.':q'.$questionid,
								'minor_type_code'	=> $data['type_code'],
								'value'				=> '',
								'link_type'			=> SQ_LINK_TYPE_2,
								'is_dependant'		=> TRUE,
								'is_exclusive'		=> FALSE,
								'sort_order'		=> $new_sort_order,
								'locked'			=> 0,
							   );
				}
				$new_sort_order++;
			}//end foreach
		}//end if

		if (is_null($sort_by)) $sort_by = 'sort_order';

		uasort($links, create_function('$a,$b','return $a["'.$sort_by.'"] > $b["'.$sort_by.'"];'));

		return $links;

	}//end getLinks()


	/**
	* Used by asset map to return a asset map-style link array
	*
	* @return array
	* @access public
	*/
	function getAssetMapLinks()
	{
		$new_sort_order = 0;
		$old_links = $this->getLinks($this->id, SQ_SC_LINK_BACKEND_NAV);
		$links = Array();
		while (!empty($old_links)) {
			array_push($links, array_shift($old_links));
		}

		$questions = $this->attr('questions');

		foreach (array_keys($links) as $i) {

			$link = &$links[$i];
			if ($link['minor_type_code'] == 'bodycopy') {
				unset($links[$i]);
				continue;
			}

			if ($link['minor_type_code'] == 'simple_form') {
				unset($links[$i]);
				continue;
			}

			// mould it all to the asset map's liking
			$link['url'] = '';
			$link['path'] = '';
			$link['num_kids'] = 0;
			$link['accessible'] = 1;

			$link['assetid'] = $link['minorid'];
			$link['type_code'] = $link['minor_type_code'];
			$link['linkid'] = $link['majorid'].':'.$link['minorid'];

			$questionid = str_replace($this->id.':q', '', $link['assetid']);

			// make name and short name the same
			$link['name'] = $questions[$questionid]['attributes']['name'];
			$link['short_name'] = $link['name'];

			// make the status the same as the form's one
			$link['status'] = $this->status;

			unset($link['minor_type_code']);
			unset($link['majorid']);
			unset($link['minorid']);
			unset($link['value']);
			unset($link['is_exclusive']);

		}//end foreach

		return $links;

	}//end getAssetMapLinks()


	/**
	* Get all asset ids that are above the passed assetid in the various trees in which it exists
	*
	* @param int			$shadowid			the shadow ID of the shadow asset to get (ie. the
	*											part after the ':')
	* @param string|array	$type_code			the type of asset that is linked (eg 'User', 'User_Group', etc)
	*											if an array returns link if matches any of the array values
	* @param boolean		$strict_type_code	whether we are finding assets that are just a $type_code
	*											or $type_code and any of it's sub-classes
	*
	* @return array
	* @access public
	* @see Asset_Manager::getParents()
	*/
	function getParents($shadowid, $type_code='', $strict_type_code=TRUE)
	{
		// basically get the parents of the section, and add itself in the
		$ret_val = $GLOBALS['SQ_SYSTEM']->am->generateGetParentsQuery($this->id, $type_code, $strict_type_code);
		if (empty($ret_val)) return Array();

		$queried_parents = NULL;
		try {
			$query = MatrixDAL::preparePdoQuery(implode(' ', $ret_val['sql_array']));
			foreach ($ret_val['bind_vars'] as $bind_var => $bind_value) {
				MatrixDAL::bindValueToPdo($query, $bind_var, $bind_value);
			}

			$queried_parents = MatrixDAL::executePdoAll($query);
		} catch (Exception $e) {
			throw new Exception($e->getMessage());
		}//end

		$parents = Array();

		foreach ($queried_parents as $queried_parent) {
			$parents[$queried_parent['majorid']] = $queried_parent['type_code'];
		}

		$parents[$this->id] = $this->type();

		return $parents;

	}//end getParents()


	/**
	* Get all asset ids that are below the passed assetid in the various trees in which it exists
	*
	* @param int			$assetid			the id of the asset to get its children for
	* @param string|array	$type_code			the type of asset that is linked
	*											(eg 'Page', 'File', etc)
	*											if an array returns link if matches any
	*											of the array values
	* @param boolean		$strict_type_code	whether we are finding assets that are just a $type_code
	*											or $type_code and any of it's sub-classes
	* @param boolean		$dependant			if TRUE, results will be filtered to assets that are dependants.
	*											If FALSE, results will all not be dependants.
	*											If NULL, results will not be filtered
	* @param string			$sort_by			a field in the asset table you would like the results
	*											sorted by (eg. name, short_name etc)
	*
	* @return array
	* @access public
	* @see Asset_Manager::getChildren()
	*/
	function getChildren($assetid, $type_code='', $strict_type_code=TRUE, $dependant=NULL, $sort_by=NULL)
	{
		// no shadow assets have children in this asset
		if (!is_numeric($assetid)) return Array();

		if (!is_array($type_code)) {
			if (empty($type_code)) {
				$type_code = Array();
			} else {
				$type_code = Array($type_code);
			}
		}

		$children = Array();

		// this is so we get the sections as we go
		// have to do this all in here because calling asset manager will bring on a vicious loop
		$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
		if (($asset instanceof Bridge) && ($assetid != $this->id)) {
			$children = $asset->getChildren($assetid, $type_code, $strict_type_code, $dependant, $sort_by);
			$this->forgetAsset($asset);
		} else {
			$ret_val = $GLOBALS['SQ_SYSTEM']->am->generateGetChildrenQuery($asset, $type_code, $strict_type_code, $dependant, $sort_by);
			unset($asset);


			if (!empty($ret_val)) {
				$sql_array = $ret_val['sql_array'];
				$bind_vars = $ret_val['bind_vars'];

				$result = NULL;
				try {
					$query = MatrixDAL::preparePdoQuery(implode(' ', $sql_array));
					foreach ($bind_vars as $bind_var => $bind_value) {
						MatrixDAL::bindValueToPdo($query, $bind_var, $bind_value);
					}
					$result = MatrixDal::executePdoGroupedAssoc($query);
				} catch (Exception $e) {
					throw new Exception($e->getMessage());
				}

				$children = $result;
			}
		}

		// now questions
		$entries = $this->attr('questions');
		if (empty($type_code)) {
			foreach ($entries as $questionid => $data) {
				$children[$this->id.':q'.$questionid] = Array(Array('type_code' => $data['type_code']));
			}
		} else {
			foreach ($type_code as $this_type_code) {
				foreach ($entries as $questionid => $data) {
					if ($strict_type_code) {
						if ($data['type_code'] == $this_type_code) {
							$children[$this->id.':q'.$questionid] = Array(Array('type_code' => $data['type_code']));
						}
					} else {
						$type_desc = $GLOBALS['SQ_SYSTEM']->am->getTypeDescendants($this_type_code) + Array($this_type_code);
						if (in_array($data['type_code'], $type_desc)) {
							$children[$this->id.':q'.$questionid] = Array(Array('type_code' => $data['type_code']));
						}
					}
				}
			}
		}

		if (!is_null($sort_by)) {
			uasort($children, create_function('$a,$b','return $a["'.$sort_by.'"] > $b["'.$sort_by.'"]'));
		}

		return $children;

	}//end getChildren()


	/**
	* Returns an array of assetid's + info in the order that they propogate out from THIS BRIDGE'S url
	*
	* @param string	$assetid	the id of the last asset in the lineage
	* @param string	$protocol	the protocol to match -> NULL means it is ignored
	* @param string	$url		the url to check for -> NULL defaults it to current url
	*
	* @return array
	* @access public
	*/
	function getLineageFromURL($assetid, $protocol, $url)
	{
		return Array();

	}//end getLineageFromURL()


//--        ASSET COMMON FUNCTIONS        --//


	/**
	* Returns name of the asset
	*
	* @param boolean	$short_name	whether or not we are after the shortname or the full name
	* @param int		$contextid	what context to return the name from
	*
	* @return array
	* @access private
	* @see Asset::_getName()
	*/
	function _getName($short_name=FALSE, $contextid=NULL)
	{
		// No context specified, using the current context
		if ($contextid === NULL) {
			$contextid = $GLOBALS['SQ_SYSTEM']->getContextId();
		}//end if

		// Obtain the attribute value for Name from the specified Context
		$values = $GLOBALS['SQ_SYSTEM']->am->getAttributeValuesByName('name', $this->type(), Array($this->id), $contextid);
		if (empty($values) === TRUE) {
			return parent::_getName($short_name, $contextid);
		} else {
			return $values[$this->id];
		}

	}//end _getName()


	/**
	* Returns an array of all the permitted link type, the type asset and the cardinality
	*
	* @return array
	* @access private
	* @see Asset::_getAllowLinks()
	*/
	function _getAllowedLinks()
	{
		$links = Array(
					SQ_LINK_TYPE_2	=> Array(
										'bodycopy'		=> Array(
															'card'		=> '1',
															'exclusive'	=> FALSE,
														   ),
										'simple_form'	=> Array(
															'card'		=> 'M',
															'exclusive'	=> FALSE,
														   ),
									   ),
					SQ_LINK_TYPE_3	=> Array(
										'bodycopy'	=> Array(
														'card'		=> '1',
														'exclusive'	=> FALSE,
													   ),
									   ),
				 );
		return $links;

	}//end _getAllowedLinks()


//--        QUESTION HANDLING FUNCTIONS        --//


	/**
	* Creates a question and adds it to the form
	*
	* @param string	$type_code	the type code of the question we want to add
	* @param string	$name		the name of the new question
	*
	* @return boolean
	* @access public
	*/
	function createQuestion($type_code, $name=NULL)
	{
		$next_question = $this->attr('next_question');
		$questions = $this->attr('questions');

		// if we weren't passed a name for some reason, set a default based on
		// the 'next question' counter
		if (is_null($name)) {
			$name = 'Question '.$next_question;
		}

		// set up the new question and bump the 'next question' counter
		$questions[$next_question] = Array(
										'type_code'		=> $type_code,
										'name'			=> $name,
										'sort_order'	=> count($questions),
										'attributes'	=> Array(
															'name'	=> $name,
														   ),
									 );

		$this->setAttrValue('questions', $questions);
		$this->setAttrValue('next_question', $next_question + 1);

		// for system integrity purposes, acquire the lock on the new question
		$GLOBALS['SQ_SYSTEM']->am->acquireLock($this->id.':q'.$next_question, 'all', $this->id, TRUE);
		$GLOBALS['SQ_SYSTEM']->am->updateLock($this->id, 'all');

		return TRUE;

	}//end createQuestion()


	/**
	* Deletes a question
	*
	* @param string	$assetid	the id of the question to delete
	*
	* @return boolean
	* @access public
	*/
	function deleteQuestion($assetid)
	{
		list($sectionid, $shadowid) = explode(':q', $assetid);

		// make sure we can actually do something this
		if ($sectionid != $this->id) {
			// TRANSLATE: Get an error number for this
			trigger_error('Cannot delete question '.$assetid.' from '.translate('asset_format', $this->_getName(), $this->id).' as it does not belong to this section', E_USER_WARNING);
			return FALSE;
		}

		$questions = $this->attr('questions');

		if (!isset($questions[$shadowid])) {
			// TRANSLATE: Get an error number for this
			trigger_error('Cannot delete question '.$assetid.' from '.translate('asset_format', $this->_getName(), $this->id).' as it does not exist', E_USER_WARNING);
			return FALSE;
		} else {
			unset($questions[$shadowid]);
		}

		$this->setAttrValue('questions', $questions);
		return TRUE;

	}//end deleteQuestion()


	/**
	* Calls a specified question to paint itself
	*
	* @param string	$assetid	the (shadow) asset ID of the question we need to paint
	*
	* @return void
	* @access public
	*/
	function paintQuestion($assetid)
	{
		$q = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
		echo $q->getHTMLField();

	}//end paintQuestion()


//--        FRONT-END PROCESSING FUNCTIONS        --//


	/**
	* Processes the form
	*
	* @return boolean
	* @access public
	*/
	function process()
	{
		if (!isset($this->_is_processed)) {
			$this->_errors = Array();
			$this->_answers = Array();

			foreach ($this->attr('questions') as $shadowid => $data) {
				// get the question
				$assetid = $this->id.':q'.$shadowid;
				$name = 'q'.$assetid;
				$q = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
				$q->populate($this);

				// set the value and check whether it's valid
				if (isset($_POST[$name])) $q->setValue($_POST[$name]);
				$valid = $q->hasValidValue();

				if (!$valid) $this->_errors = array_merge($this->_errors, $q->getErrors());

				$this->_answers[$assetid] = Array(
												'name'			=> $data['name'],
												'type_code'		=> $data['type_code'],
												'value'			=> $q->getValue(),
												'summary'		=> $q->getSummary(),
												'extra_data'	=> $q->getExtraData(),
												'is_error'		=> !$valid,
											);

				$GLOBALS['SQ_SYSTEM']->am->forgetAsset($q);
				unset($q);

			}//end foreach question

			// Now, handle nested forms
			foreach ($GLOBALS['SQ_SYSTEM']->am->getLinks($this->id, SQ_LINK_TYPE_2, 'simple_form', FALSE) as $link) {
				$sectionid = $link['minorid'];
				$s = $GLOBALS['SQ_SYSTEM']->am->getAsset($sectionid);

				$valid = $s->process();
				if (!$valid) {
					$this->_errors = array_merge($this->_errors, $s->getErrors());
				}

				$this->_answers += array_merge($this->_answers, $s->getAnswers());

				$GLOBALS['SQ_SYSTEM']->am->forgetAsset($s);
				unset($s);
			}

			$this->_is_processed = TRUE;
		}//end if

		return empty($this->_errors);

	}//end process()


	/**
	* Return a list of errors from last time we processed the form
	*
	* @return array
	* @access public
	*/
	function getErrors()
	{
		return $this->_errors;

	}//end getErrors()


	/**
	* Return a list of full question information
	*
	* Returned array is in the following format:
	* <pre>
	* Array(
	* 	assetid	=> Array(
	* 				'name'			=> string,		// name of question
	* 				'type_code'		=> string,		// type code of question
	*				'sort_order'	=> int			// sort order (for asset map)
	* 			   ),
	* )
	* </pre>
	*
	* @param boolean	$include_nested	if TRUE, return all the questions underneath this form (including those belonging to nested forms); otherwise, only return questions attached to this form
	*
	* @return array
	* @access public
	*/
	function getQuestions($include_nested=FALSE)
	{
		$am = $GLOBALS['SQ_SYSTEM']->am;
		$questions = Array();

		$questions_raw = $this->attr('questions');
		foreach ($questions_raw as $q_id => $question) {
			$new_id = $this->id.':q'.$q_id;
			$questions[$new_id] = $question;

			// clear out unnecessary info from the question info
			unset($questions[$new_id]['attributes']);
		}

		if ($include_nested) {
			$nested_forms = $am->getLinks($this->id, SQ_SC_LINK_SIGNIFICANT, 'simple_form', FALSE);
			foreach ($nested_forms as $link) {
				$form = $am->getAsset($link['minorid']);
				if (!is_null($form)) {
					$questions = array_merge($questions, $form->getQuestions($include_nested));
					$am->forgetAsset($form);
				}
			}
		}

		return $questions;

	}//end getQuestions()


	/**
	* Return a list of full answer information recorded from processed questions
	*
	* Returned array is in format:
	* <pre>
	* Array(
	* 	assetid => Array(
	*				'name'			=> string,		// name of question
	*				'type_code'		=> string,		// type code of question
	*				'value'			=> mixed,		// raw value
	*				'summary'		=> string,		// question summary text
	*				'extra_data'	=> mixed,		// free-form metadata for result
	*				'is_error'		=> boolean,		// did this error out?
	*			   ),
	* )
	* </pre>
	*
	* @return array
	* @access public
	*/
	function getAnswers()
	{
		return $this->_answers;

	}//end getAnswers()


	/**
	* Return a list of raw answer values recorded from processed questions
	*
	* @return array
	* @access public
	*/
	function getAnswerValues()
	{
		$answer_values = Array();
		foreach ($this->_answers as $assetid => $data) {
			$answer_values[$assetid] = $data['value'];
		}
		return $answer_values;

	}//end getAnswerValues()


	/**
	* Return a list of answer summaries recorded from processed questions
	*
	* @return array
	* @access public
	*/
	function getAnswerSummaries()
	{
		$answer_values = Array();
		foreach ($this->_answers as $assetid => $data) {
			$answer_values[$assetid] = $data['summary'];
		}
		return $answer_values;

	}//end getAnswerSummaries()


	/**
	* Return a list of answer extra data recorded from processed questions
	*
	* @return array
	* @access public
	*/
	function getAnswerExtraData()
	{
		$answer_values = Array();
		foreach ($this->_answers as $assetid => $data) {
			$answer_values[$assetid] = $data['extra_data'];
		}
		return $answer_values;

	}//end getAnswerExtraData()


//--        CLIENT-SIDE VALIDATION        --//


	/**
	* Returns the client-side validation code to be placed before the form
	*
	* Note that this is only defined in the simple form and not the section, as
	* a form should be validated as a whole
	* The return value of this function should be placed in a function that acts
	* as an onSubmit handler. Any text relating to failed rules will be placed
	* in the submission_errors array; do what you wish with those.
	* If no validation rules exist, an empty string ('') will be returned.
	*
	* @return string
	* @access public
	*/
	function getClientSideJS()
	{
		ob_start();
			?>submission_errors = new Array();

			<?php
			$init_code = ob_get_contents();
		ob_end_clean();

		$code = '';

		$questions = $this->attr('questions');
		foreach (array_keys($questions) as $key) {
			$q = $GLOBALS['SQ_SYSTEM']->am->getAsset($this->id.':q'.$key);

			$q_code = $q->generateJSCode();
			$code .= $q_code.(empty($q_code) ? '' : "\n");

			$GLOBALS['SQ_SYSTEM']->am->forgetAsset($q);
			unset($q);
		}

		foreach ($GLOBALS['SQ_SYSTEM']->am->getLinks($this->id, SQ_LINK_TYPE_2, 'simple_form', FALSE) as $link) {
			// get the section
			$sectionid = $link['minorid'];
			$s = $GLOBALS['SQ_SYSTEM']->am->getAsset($sectionid);
			$section_questions = $s->attr('questions');

			foreach (array_keys($section_questions) as $key) {
				$q = $GLOBALS['SQ_SYSTEM']->am->getAsset($s->id.':q'.$key);

				$q_code = $q->generateJSCode();
				$code .= $q_code.(empty($q_code) ? '' : "\n");

				$GLOBALS['SQ_SYSTEM']->am->forgetAsset($q);
				unset($q);
			}

			$GLOBALS['SQ_SYSTEM']->am->forgetAsset($s);
			unset($s);
		}

		if (empty($code)) {
			return '';
		} else {
			return $init_code.$code;
		}

	}//end getClientSideJS()


	/**
	* Builds a JavaScript fn that can be called to initiate client-side validation
	*
	* @return string
	* @access public
	*/
	function getClientSideFunction()
	{
		$code = $this->getClientSideJS();
		if (empty($code)) return '';

		ob_start();
			?>
<script type="text/javascript">
function beforeSubmit_<?php echo $this->getPrefix() ?>(form) {
	<?php echo $code; ?>

	if (submission_errors.length > 0) {
		var errors_list = "<?php echo translate('core_form_submission_errors'); ?>\n";
		for(x in submission_errors) {
			errors_list += submission_errors[x] + "\n";
		}

		alert(errors_list);
		return false;
	} else {
		return true;
	}
}//end beforeSubmit_<?php echo $this->getPrefix() ?>()
</script>
			<?php
			$code = ob_get_contents();
		ob_end_clean();

		return $code;

	}//end getClientSideFunction()


//--        KEYWORD PROVISION        --//


	/**
	* Add valid keywords for this asset to an array of keywords when asked
	*
	* @param object	&$broadcaster	the asset that triggered the event
	* @param array	$vars			the vars that get submitted by the broadcaster
	*								we add keywords to the $vars['keywords'] array
	*
	* @return boolean
	* @access private
	*/
	function onRequestKeywords(&$broadcaster, $vars=Array())
	{
		$vars['keywords'] = isset($vars['keywords']) ? $vars['keywords'] : Array();
		$keywords = Array();

		if ($this->isFormatBodycopyEnabled()) {
			$keywords = $this->getDisplayKeywords('form_');
		}

		$vars['keywords'] = array_merge($vars['keywords'], $keywords);

	}//end onRequestKeywords()


	/**
	* Returns a list of display keywords for the questions handled by this section
	*
	* return format is (string => string)	keyword => description
	*
	* @param string	$prefix	the prefix that should be applied to each
	*						keyword returned by this function
	*
	* @return array
	* @access public
	*/
	function getDisplayKeywords($prefix)
	{
		$questions = $this->attr('questions');
		$keywords = Array();

		foreach ($questions as $shadowid => $question) {
			$keywords[$prefix.$this->id.'_q'.$shadowid] = 'Question Field: '.ellipsisize($question['attributes']['name'], 30).' ('.$this->id.':q'.$shadowid.')';
			$keywords[$prefix.$this->id.'_q'.$shadowid.'_note'] = 'Question Note: '.ellipsisize($question['attributes']['name'], 30).' ('.$this->id.':q'.$shadowid.')';
			$keywords[$prefix.$this->id.'_q'.$shadowid.'_id'] = 'Question ID: '.ellipsisize($question['attributes']['name'], 30).' ('.$this->id.':q'.$shadowid.')';
		}

		foreach ($GLOBALS['SQ_SYSTEM']->am->getLinks($this->id, SQ_LINK_TYPE_2, 'simple_form', FALSE) as $link) {
			// get the section
			$sectionid = $link['minorid'];
			$s = $GLOBALS['SQ_SYSTEM']->am->getAsset($sectionid);

			$keywords += $s->getDisplayKeywords($prefix);
			$keywords[$prefix.$s->id.'_name'] = 'Nested Form Name: '.$s->name.' ('.$s->id.')';

			$GLOBALS['SQ_SYSTEM']->am->forgetAsset($s);
			unset($s);
		}

		$keywords['submit_button'] = 'Submit Button';
		$keywords['reset_button'] = 'Reset Button';
		$keywords['form_errors'] = 'Form Errors';

		return $keywords;

	}//end getDisplayKeywords()


	/**
	* Returns a list of response keywords for the questions handled by this section
	*
	* Return format is (string => string)	keyword => description
	*
	* @param string	$prefix	the prefix that should be applied to each
	*						keyword returned by this function
	*
	* @return array
	* @access public
	*/
	function getResponseKeywords($prefix)
	{
		$questions = $this->attr('questions');
		$keywords = Array();

		foreach ($questions as $shadowid => $question) {
			$keywords[$prefix.$this->id.'_q'.$shadowid] = 'Summary: '.ellipsisize($question['attributes']['name'], 30).' ('.$this->id.':q'.$shadowid.')';
			$keywords[$prefix.$this->id.'_q'.$shadowid.'_raw'] = 'Raw Response: '.ellipsisize($question['attributes']['name'], 30).' ('.$this->id.':q'.$shadowid.')';
		}

		foreach ($GLOBALS['SQ_SYSTEM']->am->getLinks($this->id, SQ_LINK_TYPE_2, 'simple_form', FALSE) as $link) {
			// get the nested form
			$sectionid = $link['minorid'];
			$s = $GLOBALS['SQ_SYSTEM']->am->getAsset($sectionid);

			$keywords += $s->getResponseKeywords($prefix);
			$keywords[$prefix.$s->id.'_name'] = 'Nested Form Name: '.$s->name.' ('.$s->id.')';

			$GLOBALS['SQ_SYSTEM']->am->forgetAsset($s);
			unset($s);
		}

		return $keywords;

	}//end getResponseKeywords()


//--        KEYWORD REPLACEMENTS PROVISION        --//


	/**
	* Returns a list of display keywords for the questions handled by this section
	*
	* Return format is (string => string)	keyword => description
	*
	* @param string	$prefix	the prefix that should be applied to each
	*						keyword returned by this function
	*
	* @return array
	* @access public
	*/
	function getDisplayKeywordReplacement($prefix)
	{
		$questions = $this->attr('questions');
		$keywords = Array();

		foreach ($questions as $shadowid => $question) {
			// get the question
			$q = $GLOBALS['SQ_SYSTEM']->am->getAsset($this->id.':q'.$shadowid);
			$keywords[$prefix.$this->id.'_q'.$shadowid] = $q->getHtmlField();
			$keywords[$prefix.$this->id.'_q'.$shadowid.'_note'] = $q->attr('note');
			$keywords[$prefix.$this->id.'_q'.$shadowid.'_id'] = 'q'.$this->id.'_q'.$shadowid;
			$GLOBALS['SQ_SYSTEM']->am->forgetAsset($q);
			unset($q);
		}

		foreach ($GLOBALS['SQ_SYSTEM']->am->getLinks($this->id, SQ_LINK_TYPE_2, 'simple_form', FALSE) as $link) {
			// get the nested form
			$sectionid = $link['minorid'];
			$s = $GLOBALS['SQ_SYSTEM']->am->getAsset($sectionid);

			$keywords += $s->getDisplayKeywordReplacements($prefix);
			$keywords[$prefix.$s->id.'_name'] = $s->name;

			$GLOBALS['SQ_SYSTEM']->am->forgetAsset($s);
			unset($s);
		}

		// Submit Button, Reset Button, Form Errors
		ob_start();
			submit_button('submit_button', $this->attr('submit_button'));
			$submit_button = ob_get_contents();
		ob_end_clean();

		ob_start();
			reset_button('reset_button', $this->attr('reset_button'));
			$reset_button = ob_get_contents();
		ob_end_clean();

		$keywords['submit_button'] = $submit_button;
		$keywords['reset_button']  = $reset_button;
		$keywords['form_errors']   = implode('<br/>', $this->_errors);

		return $keywords;

	}//end getDisplayKeywordReplacement()


	/**
	* Returns a list of response keywords for the questions handled by this section
	*
	* Return format is (string => string)	keyword => description
	*
	* @param string	$prefix	the prefix that should be applied to each
	*						keyword returned by this function
	*
	* @return array
	* @access public
	*/
	function getResponseKeywordReplacement($prefix)
	{
		$questions = $this->attr('questions');
		$keywords = Array();

		foreach ($questions as $shadowid => $question) {
			$q = $GLOBALS['SQ_SYSTEM']->am->getAsset($this->id.':q'.$shadowid);
			$keywords[$prefix.$this->id.'_q'.$shadowid] = htmlentities($q->getSummary(), ENT_NOQUOTES, SQ_CONF_DEFAULT_CHARACTER_SET);
			$keywords[$prefix.$this->id.'_q'.$shadowid.'_raw'] = htmlentities($q->getValue(), ENT_NOQUOTES, SQ_CONF_DEFAULT_CHARACTER_SET);
			$GLOBALS['SQ_SYSTEM']->am->forgetAsset($q);
			unset($q);
		}

		foreach ($GLOBALS['SQ_SYSTEM']->am->getLinks($this->id, SQ_LINK_TYPE_2, 'simple_form', FALSE) as $link) {
			// get the nested form
			$sectionid = $link['minorid'];
			$s = $GLOBALS['SQ_SYSTEM']->am->getAsset($sectionid);

			$keywords += $s->getResponseKeywordReplacements($prefix);
			$keywords[$prefix.$s->id.'_name'] = $s->name;

			$GLOBALS['SQ_SYSTEM']->am->forgetAsset($s);
			unset($s);
		}

		return $keywords;

	}//end getResponseKeywordReplacement()


//--        CUSTOM FORMAT BODYCOPY FUNCTIONS        --//


	/**
	* Creates the format bodycopy
	*
	* Returns false if bodycopy already exists
	*
	* @param boolean	$enable_on_create	if true, will set the bodycopy to be enabled
	*										(TYPE_2 link) when it is created. If false,
	*										creates it disabled (TYPE_3 link).
	*
	* @return boolean
	* @access public
	*/
	function createFormatBodycopy($enable_on_create=TRUE)
	{
		$_bodycopy_link = $GLOBALS['SQ_SYSTEM']->am->getLinks($this->id, SQ_LINK_TYPE_2 | SQ_LINK_TYPE_3, 'bodycopy', 'format');
		$bodycopy_link = reset($_bodycopy_link);

		// we already have a bodycopy link?!
		if ($bodycopy_link) {
			return FALSE;
		} else {
			$GLOBALS['SQ_SYSTEM']->am->includeAsset('bodycopy');

			$link_type = ($enable_on_create ? SQ_LINK_TYPE_2 : SQ_LINK_TYPE_3);

			$asset = new Bodycopy();
			$copy_link = Array(
							'asset'			=> &$this,
							'value'			=> 'format',
							'link_type'		=> $link_type,
							'is_dependant'	=> 1,
							'is_exclusive'	=> 1,
						 );

			$asset->setAttrValue('name', 'Format Bodycopy');
			if (!$asset->create($copy_link)) return FALSE;

			$GLOBALS['SQ_SYSTEM']->am->forgetAsset($asset);
			unset($asset);
		}

		return TRUE;

	}//end createFormatBodycopy()


	/**
	* Returns the format bodycopy or NULL
	*
	* @param boolean	$only_if_enabled	if true, will return NULL if the bodycopy is
	*										disabled. If false, will only return NULL if
	*										never created.
	*
	* @return object
	* @access public
	*/
	function &getFormatBodycopy($only_if_enabled=TRUE)
	{
		$asset = NULL;

		if ($only_if_enabled) {
			$link_types = SQ_LINK_TYPE_2;
		} else {
			$link_types = SQ_LINK_TYPE_2 | SQ_LINK_TYPE_3;
		}

		$tmp_bodycopy = $GLOBALS['SQ_SYSTEM']->am->getLinks($this->id, $link_types, 'bodycopy', 'format');
		$bodycopy_link = reset($tmp_bodycopy);

		if ($bodycopy_link) {
			$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($bodycopy_link['minorid'], 'bodycopy');
		}

		return $asset;

	}//end getFormatBodycopy()


	/**
	* Returns whether the format bodycopy is enabled
	*
	* @return boolean
	* @access public
	*/
	function isFormatBodycopyEnabled()
	{
		$link_types = SQ_LINK_TYPE_2 | SQ_LINK_TYPE_3;

		$_bodycopy_link = $GLOBALS['SQ_SYSTEM']->am->getLinks($this->id, $link_types, 'bodycopy', 'format');
		$bodycopy_link = reset($_bodycopy_link);

		if ($bodycopy_link) {
			return ($bodycopy_link['link_type'] == SQ_LINK_TYPE_2);
		} else {
			return FALSE;
		}


	}//end isFormatBodycopyEnabled()


	/**
	* Handles the enabling or disabling of the format bodycopy
	*
	* @param boolean	$enabled	set to true to enable the format bodycopy,
	*								set to false to disable it
	*
	* @return boolean
	* @access public
	*/
	function setUseFormatBodycopy($enabled)
	{
		$tmp_bodycopy = $GLOBALS['SQ_SYSTEM']->am->getLinks($this->id, SQ_LINK_TYPE_2 | SQ_LINK_TYPE_3, 'bodycopy', 'format');
		$bodycopy_link = reset($tmp_bodycopy);

		if (!$bodycopy_link) {
			// no bodycopy yet? If we're trying to set to disabled, then we don't
			// need to do anything - if not then we need to create it
			if ($enabled) {
				if (!$this->createFormatBodycopy()) return FALSE;
			}
		} else {
			// set link type to either TYPE_2 if enabled or TYPE_3 if disabled
			$new_link_type = ($enabled) ? SQ_LINK_TYPE_2 : SQ_LINK_TYPE_3;
			if ($bodycopy_link['link_type'] != $new_link_type) {
				$GLOBALS['SQ_SYSTEM']->am->updateLink($bodycopy_link['linkid'], $new_link_type);
			}
		}

		return TRUE;

	}//end setUseFormatBodycopy()


//--        FORM SELF-PRINTING FUNCTIONS        --//


	/**
	* Paint the form
	*
	* @param boolean	$print_form_tags	print out the form tags
	* @param boolean	$submit_buttons		whether to paint the submit and cancel buttons
	* @param boolean	$print_js			print the client-side validation function (if any)
	*
	* @return void
	* @access public
	*/
	function printBody($print_form_tags=TRUE, $submit_buttons=TRUE, $form_action='', $print_js=TRUE)
	{
		// return custom bodycopy but ONLY if enabled
		$format_bodycopy = $this->getFormatBodycopy(TRUE);

		// default form action
		if (empty($form_action)) {
			$form_action = current_url(TRUE, TRUE, FALSE);
		}
		
		// We need to print js function even if we are not printing the form here, as another form in higher level (if any) might wanna use it
		if ($print_js && $this->attr('client_side')) {
			$client_side = $this->getClientSideFunction('beforeSubmit_'.$this->getPrefix());
			$onsubmit = ' onSubmit="return beforeSubmit_'.$this->getPrefix().'(this)"';
		} else {
			$client_side = '';
			$onsubmit = '';
		}
		echo $client_side;
		
		if ($print_form_tags) {
			?><form id="<?php echo $this->getPrefix() ?>" enctype="multipart/form-data" action="<?php echo $form_action; ?>" method="post"<?php echo $onsubmit;  ?>><?php
		}

		// if format bodycopy enabled, print with that, otherwise print default
		if ($format_bodycopy) {
			$this->printCustomFormat($format_bodycopy);
		} else {
			$this->printDefaultFormat($submit_buttons);
		}

		if ($print_form_tags) {
			?></form><?php
		}

	}//end printBody()


	/**
	* Paints the form using a default format
	*
	* @param boolean	$submit_buttons	whether to paint the submit and cancel buttons
	*
	* @return void
	* @access private
	*/
	function printDefaultFormat($submit_buttons=TRUE)
	{
		?>
		<table>
		<?php foreach ($this->attr('questions') as $shadowid => $question) {
			$q = $GLOBALS['SQ_SYSTEM']->am->getAsset($this->id.':q'.$shadowid);
			$name = array_get_index($question['attributes'], 'name', '');
			$note = array_get_index($question['attributes'], 'note', '');

			$field = $q->getHTMLField();
			?>
			<tr>
				<td><?php echo $name;
				echo ($note == '') ? '' : ('<br/><span style="font-size:0.8em">'.$note.'</span>'); ?></td>
				<td><?php echo $field; ?></td>
			</tr>
		<?php
			$GLOBALS['SQ_SYSTEM']->am->forgetAsset($q);
			unset($q);
		}

		if ($submit_buttons) {
			?>
			<tr><td style="text-align:center" colspan="2"><?php
				submit_button('submit_button', $this->attr('submit_button'));
				reset_button('reset_button', $this->attr('reset_button'));
			?></td></tr>
		<?php }
		?>
		</table>
		<?php

	}//end printDefaultFormat()


	/**
	* Paints the form using the custom format bodycopy
	*
	* @param object	&$bodycopy	the custom format bodycopy to be used
	*
	* @return void
	* @access private
	*/
	function printCustomFormat(&$bodycopy)
	{
		$replacements = $this->getDisplayKeywordReplacement('form_');
		$bodycopy->setKeywordReplacements($replacements);
		$bodycopy->printBody();

	}//end printCustomFormat()


//--        FORM STANDARD RECEIPT FUNCTIONS        --//


	/**
	* Print a standard receipt format
	*
	* @param boolean	$html				true - (default) to use HTML format
	*										false - use text format
	* @param boolean	$include_form_info	set to true to include form information
	*										(if false, the "Form Response" header
	*										will not be printed)
	*
	* @return void
	* @access public
	*/
	function printReceipt($html=TRUE, $include_form_info=TRUE)
	{
		$answers = Array();

		// include form info if asked for
		if ($include_form_info) {
			$form_info = Array();
			$lineage = $GLOBALS['SQ_SYSTEM']->am->getLineageFromURL();
			$lineage_names = Array();
			foreach ($lineage as $asset) {
				$lineage_names[] = $asset['name'];
			}

			$form_info[] = Array(
							'name'	=> 'Form Submitted',
							'value'	=> $this->name.' (Id: #'.$this->id.')',
						   );

			$form_info[] = Array(
							'name'			=> 'Form URL',
							'value'			=> current_url(TRUE, TRUE),
							'html_value'	=> '<a href="'.current_url(TRUE, TRUE).'">'.current_url(TRUE, TRUE).'</a>',
						   );

			$form_info[] = Array(
							'name'	=> 'Form Lineage',
							'value'	=> implode(' => ', $lineage_names),
						   );

			$form_info[] = Array(
							'name'	=> 'Time of Submission',
							'value'	=> date('l, j F Y H:i:s \G\M\TO'),
						   );

			if ($GLOBALS['SQ_SYSTEM']->userPublic()) {
				$form_info[] = Array(
								'name'	=> 'Submitted By',
								'value'	=> 'No logged in user',
							   );
			} else {
				$user = $GLOBALS['SQ_SYSTEM']->user;
				$form_info[] = Array(
								'name'	=> 'Submitted By',
								'value'	=> $user->name.' (Id: #'.$user->id.')',
							   );
			}

			// set max length equal to "time of submission" (we are getting the
			// max length of a question so the answers will align in the text
			// version)
			$max_name_length = 18;
		} else {
			$max_name_length = 0;
		}

		// gather questions
		foreach ($this->attr('questions') as $shadowid => $question) {
			$q = $GLOBALS['SQ_SYSTEM']->am->getAsset($this->id.':q'.$shadowid);
			$name = array_get_index($question['attributes'], 'name', '');
			$summary = $q->getSummary();

			$answers[] = Array(
							'name'	=> $name,
							'value'	=> $summary,
						 );
			$max_name_length = max($max_name_length, strlen($name));

			$GLOBALS['SQ_SYSTEM']->am->forgetAsset($q);
			unset($q);
		}

		// now print the receipt
		if ($html) {
			?>
			<table>
			<?php
			// only show form info if asked for
			if ($include_form_info) {
				?>
				<tr><td colspan="2"><strong>Form Information</strong></td></tr>
				<?php foreach ($form_info as $form_info_item) {
					?>
					<tr>
						<td valign="top"><ul><li><?php echo htmlspecialchars($form_info_item['name']); ?>:</li></ul></td>
						<td valign="top"><?php echo array_get_index($form_info_item, 'html_value', htmlspecialchars($form_info_item['value'])); ?></td>
					</tr>
				<?php
				}
				?><tr><td colspan="2"><strong>Form Responses</strong></td></tr><?php
			}
			?>

			<?php foreach ($answers as $answer) {
				?>
				<tr>
					<td valign="top"><ul><li><?php echo htmlspecialchars($answer['name']); ?>:</li></ul></td>
					<td valign="top"><?php echo htmlspecialchars($answer['value']); ?></td>
				</tr>
			<?php }
			?>
			</table>
			<?php
		} else {

			// only show form info if asked for
			if ($include_form_info) {
				echo '=== Form Information ==='."\n";
				foreach ($form_info as $form_info_item) {
					echo '* '.str_pad($form_info_item['name'], $max_name_length + 1).': '.$form_info_item['value']."\n";
				}
				echo "\n";
				echo '=== Form Responses ==='."\n";
			}

			foreach ($answers as $answer) {
				echo '* '.str_pad($answer['name'], $max_name_length + 1).': '.$answer['value']."\n";
			}
		}

	}//end printReceipt()


	/**
	* Remove a link by id
	*
	* @param int		$linkid	the link id of the link to remove
	* @param boolean	$moving	TRUE if this delete is part of a move operation
	*
	* @return boolean
	* @access public
	*/
	function deleteAssetLink($linkid, $moving=FALSE)
	{
		return FALSE;

	}//end deleteAssetLink()


	/**
	* Return the number of links in the system that involve the specified asset
	*
	* Can be restricted by either link type afnd/or asset type code
	*
	* @param int		$assetid			id of the the $side_of_link asset
	* @param string		$side_of_link		Which side of the link this (the current) asset
	*										is on ('major' or 'minor')
	* @param int		$link_types			integer that can be the product of bitwise operations
	*										on the SQ_LINK_* constants
	* @param mixed		$type_code			the type of asset that is linked (eg 'Page', 'File', etc)
	*										if an array returns link if matches any
	*										of the array values
	* @param boolean	$strict_type_code	whether we are finding assets that are just a $type_code
	*										or $type_code and any of it's sub-classes
	* @param int		$ignore_linkid		ignore the link represented by this link
	*										id when returning the count
	*
	* @return int
	* @access public
	*/
	function countLinks($assetid, $side_of_link='major', $link_types=0, $type_code='', $strict_type_code=TRUE, $ignore_linkid=0)
	{
		return 0;

	}//end countLinks()


	/**
	* Get asset info for use by asset map
	*
	* @param string	$assetid	Full Asset id to get information for
	*
	* @return array
	* @access public
	*/
	function getAssetMapAssetInfo($assetid)
	{
		return Array();

	}//end getAssetMapAssetInfo()


	/**
	* Check if  form's using accessible format
	*								
	*
	* @return boolean
	* @access public
	*/
	function useAccessibleFormat()
	{
		return TRUE;
	}//end useAccessibleFormat()
	
}//end class


?>
