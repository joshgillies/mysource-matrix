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
* $Id: page_decision_tree.inc,v 1.3 2012/08/30 00:58:08 ewang Exp $
*
*/


require_once SQ_CORE_PACKAGE_PATH.'/page/page.inc';


/**
* Page_Decision_Tree
*
*
* @author  Mohamed Haidar <mhaidar@squiz.net>
* @version $Revision: 1.3 $
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Page_Decision_Tree extends Page
{

	
	/**
	* Constructor
	*
	* @param string	$assetid	the assetid of the asset to load into this object
	*
	* @return void
	* @access public
	*/
	function __construct($assetid=0)
	{
		$this->_ser_attrs = TRUE;
		parent::__construct($assetid);							 

	}//end constructor
	
	
	/**
	* Perform any additional processing required during the creation of this asset
	*
	* Pages add a default web path when they are created
	*
	* @param array	&$link	information used to create the initial link
	*
	* @return boolean
	* @access private
	*/
	protected function _createAdditional(Array &$link)
	{
		if (!parent::_createAdditional($link)) return FALSE;
		
		// add a bodycopy to this page when creating
		$GLOBALS['SQ_SYSTEM']->am->includeAsset('bodycopy');
		$GLOBALS['SQ_SYSTEM']->am->includeAsset('folder');

		$sub_assets= Array(
						'type_formats'		=> 'folder',
						'result_bodycopies'	=> 'folder',
					 );

		foreach ($sub_assets as $name => $type) {
			$asset = new $type();
			$copy_link = Array('asset' => &$this, 'value' => $name ,'link_type' => SQ_LINK_TYPE_2, 'is_dependant' => 1, 'is_exclusive' => 1);

			$asset->setAttrValue('name', ucwords(str_replace('_',' ', $name)));
			if (!$asset->create($copy_link)) return FALSE;

			$GLOBALS['SQ_SYSTEM']->am->forgetAsset($asset);
			unset($asset);
		}
		
		$asset = new Bodycopy();
		$copy_link = Array('asset' => &$this, 'value' => 'page_contents' ,'link_type' => SQ_LINK_TYPE_2, 'is_dependant' => 1, 'is_exclusive' => 1);
		$asset->setAttrValue('name', 'Page Contents');
		$args = Array('content' => $this->_getDefaultBodycopyContent('page_contents'));
		if (!$asset->create($copy_link, $args)) return FALSE;
		
		if (!$this->_createTypeFormat('decision_tree_question_type_select')) return FALSE;
		if (!$this->_createTypeFormat('decision_tree_question_type_numeric')) return FALSE;

		return $this->createResultBodycopy('Default Result Bodycopy', 'default_result_bodycopy');

	}//end _createAdditional()
	
	
	/**
	* Returns an array of all the permitted link type, the type asset and the cardinality
	*
	* @return array
	* @access private
	* @see Asset::_getAllowLinks()
	*/
	function _getAllowedLinks()
	{
		$page_links = parent::_getAllowedLinks();
		$page_links[SQ_LINK_TYPE_2]['decision_tree_question'] = Array('card' => 'M', 'exclusive' => FALSE);
		$page_links[SQ_LINK_NOTICE]['decision_tree_question'] = Array('card' => 1, 'exclusive' => FALSE);
		$page_links[SQ_LINK_TYPE_2]['bodycopy'] = Array('card' => '1', 'exclusive' => TRUE);

		return $page_links;

	}//end _getAllowedLinks()
	
	
	/**
	* Returns TRUE if we can delete the passed link, or a string with the error msg
	*
	* @param int	$linkid	the link id of the link to remove
	*
	* @return mixed boolean|string
	* @access public
	*/
	public function isDeletableLink($linkid)
	{
		// the only reason why we would object to this link being
		// deleted is if it is the bodycopy linked to us
		$bodycopy_link = $GLOBALS['SQ_SYSTEM']->am->getLink($this->id, SQ_LINK_TYPE_2, 'bodycopy', TRUE, 'page_contents', 'major', TRUE);
		if (!$GLOBALS['SQ_PURGING_TRASH'] && (!empty($bodycopy_link) && $bodycopy_link['linkid'] == $linkid)) {
			return translate('error_bodycopy_link_cannot_delete');
		}

		return parent::isDeletableLink($linkid);

	}//end isDeletableLink()
	
	
	/**
	* Paint the passed asset on the frontend
	* Override printFrontend() if you wish to pass a different asset to print
	*
	* @param Asset	&$asset	the asset to print in frontend
	* @param mixed	$design	the design to print the asset with
	*
	* @return void
	* @access public
	*/
	public function printFrontendAsset(Asset $asset, $design=NULL)
	{
		
		//we are getting an AJAX Request
		if (isset($_GET['q_id']) && isset($_GET['a_id']) && isset($_GET['pts'])){
			if (!$asset->readAccess()) {
				$GLOBALS['SQ_SYSTEM']->paintLogin(translate('login'), translate('cannot_access_asset', $asset->name));
				return;
			}
			$this->printNextStep($_GET['q_id'], $_GET['a_id'], $_GET['pts']);
			return;
		}
		
		parent::printFrontendAsset($asset, $design);
		
	}//printFrontendAsset()
	
	
	/**
	* Called by the design to print the body of this asset
	*
	* It's up to the kids to override
	*
	* @return void
	* @access public
	*/
	public function printBody()
	{
		
		$replacements = Array();
		
		$page_contents = $GLOBALS['SQ_SYSTEM']->am->getLink($this->id, SQ_LINK_TYPE_2, 'bodycopy', TRUE, 'page_contents', 'major', TRUE);
		if (empty($page_contents)) return;
		
		$bodycopy = $GLOBALS['SQ_SYSTEM']->am->getAsset($page_contents['minorid']);
		
		// check to see if the bodycopy attached to this form has any content
		if ($bodycopy && !$bodycopy->isEmpty()) {
			$keyword_list = $bodycopy->getKeywords();

			if (!empty($keyword_list)) {
				foreach ($keyword_list as $keyword) {
					// Question related keywords are found here
					$replacement = $this->getPageContentsKeywordReplacement($keyword);
					
					if (is_null($replacement)) {
						// If not picked up by first function, this will pick up
						// replacements placed in separate functions - CAPTCHA,
						// errors, for instance
						$replacement = $this->getKeywordReplacement($keyword);
					}

					if (!is_null($replacement)) {
						// We have something from one of the above
						$replacements[$keyword] = $replacement;
					}
				}
			}

			$bodycopy->setKeywordReplacements($replacements);
			$bodycopy->printBody();

		} 
	}//end printBody()
	
	
	/**
	* Called by printBody() everytime there is an AJAX request to get the next question
	*
	* @param int	$question_id	the question asset id we are answering
	* @param string	$answer			the answer value of $question_id
	* @param int	$total_points	the total points accumulated upto this point
	*
	* @return void
	* @access public
	*/
	public function printNextStep($question_id, $answer, $total_points){
		
		$question = $GLOBALS['SQ_SYSTEM']->am->getAsset($question_id);
		if (is_null($question)) return;
		
		if ($question->type() == 'decision_tree_question_type_select' && $answer === $question->attr('empty_key')) return;
		
		$destinations = $question->attr('question_destinations');
		if (empty($destinations)) return;
		
		if ($question->type() == 'decision_tree_question_type_numeric'){
			$rules = $question->attr('rules');
			foreach ($rules as $rule_index => $rule_details){
				$valid = $question->evaluate($answer, $rule_details);
				if ($valid == TRUE){
					$answer = $rule_index;
					break;
				}
			}
			if (empty($valid)) return;
		}
		
		$answer_dest = $destinations[$answer];
		$answer_pts = (!empty($answer_dest['points_value'])) ? $answer_dest['points_value'] : 0;
		//add the total points so far + the points of the answer to this question
		$accumulated_pts = $total_points + $answer_pts;
			
		if (!empty($answer_dest['next_question'])) {
			$new_question = $GLOBALS['SQ_SYSTEM']->am->getAsset($answer_dest['next_question']);
			$bodycopy = $this->getBodycopy($new_question->type(), 'type_formats');
		} else if (!empty($answer_dest['result_bodycopy'])) {
			$bodycopy = $GLOBALS['SQ_SYSTEM']->am->getAsset($answer_dest['result_bodycopy']);
		} else {
			$bodycopy_points = $this->attr('bodycopy_points');
			foreach ($bodycopy_points as $assetid => $points){
				if ($accumulated_pts >= $points['min'] && $accumulated_pts <= $points['max']){
					$bodycopy = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
					break;
				}
			}
			
			if (!isset($bodycopy)){
				//use default result bodycopy
				$bodycopy = $this->getBodycopy('default_result_bodycopy', 'result_bodycopies');
			}
		}
		
		$replacements = Array();
		// check to see if the bodycopy attached to this form has any content
		if ($bodycopy && !$bodycopy->isEmpty()) {
			$keyword_list = $bodycopy->getKeywords();
			
			if (!empty($keyword_list)) {
				foreach ($keyword_list as $keyword) {
					// Question related keywords are found here
					$replacement = NULL;
					if ($keyword == 'accumulated_points'){
						$replacements[$keyword] = $accumulated_pts;
						continue;
					}
					
					if (isset($new_question)){
						$replacement = $this->getTypeFormatKeywordReplacement($keyword, $new_question);
					}
					if (is_null($replacement)) {
						// If not picked up by first function, this will pick up
						// replacements placed in separate functions - CAPTCHA,
						// errors, for instance
						$replacement = $this->getKeywordReplacement($keyword);
					}

					if (!is_null($replacement)) {
						// We have something from one of the above
						$replacements[$keyword] = $replacement;
					}
				}
			}

			$bodycopy->setKeywordReplacements($replacements);
			if (isset($new_question)) {
				echo '<input id="q'.$new_question->id.'_points" name="q'.$new_question->id.'_points" type="hidden" value="'.$answer_pts.'" />';
				echo '<div id="q'.$new_question->id.'_wrapper" class="sq_question_wrapper">';
			} else {
				echo '<div class="sq_result_bodycopy">';
			}
			
			$bodycopy->printBody();
			echo '</div>';
		}
	}//end printNextStep()
	
	
	/**
	* Generated all the javascript code required to dynamically go through the decision path.
	* Makes use of AJAX requests.
	* The JS reset_questions() function depends on some data from the first question
	*
	* @param Decision_Tree_Question	  $question	  the first question
	*
	* @return void
	* @access public
	*/
	public function generateJS($question){
		
		$current_url = $this->getURL();
		$default_val = ($question->type() == 'decision_tree_question_type_select') ? $question->attr('empty_key') : '';
		// regex code - allow empty or leading plus or minute, digits, decimal point
		$regex_code = '/(^$)|(^[-+]?\d*\.?\d+$)/';
		
		ob_start();
		?>
		<script type="text/javascript">
		
			var accum_points=new Array();
			var curr_id = 1;
				
			function get_next_step(answer, question) 
			{	

				if (document.getElementById("q"+question).type == 'text'){
					var result = decision_tree_validate_numeric(answer);
					if (result == null) {
						alert ('You must enter a numeric answer');
						return;
					}
				}

			    //extract the question number from the wrapper DIV so we can know where to place next question
				var q_element = document.getElementById("q"+question+"_wrapper");
				var req_div = q_element.parentNode;
				curr_id = req_div.id;
				curr_id = curr_id.substring(1);
				curr_id = parseInt(curr_id);
				  
				try
				{
					xmlHttp=new XMLHttpRequest();
				}
				catch(e)
				{
					try
					{
						xmlHttp=new ActiveXObject("Microsoft.XMLHTTP");
					}
					catch(e)
					{
						alert ("Your browser does not support XMLHTTP!");
						return;
					}
				}
				
				var points = calculate_points(question);
				var url = "<?php echo $current_url ?>";
				
				xmlHttp.onreadystatechange = question_state_changed;
				url=url+'?a_id='+answer+'&q_id='+question+'&pts='+points;
				xmlHttp.open("GET", url, true);
				xmlHttp.send(null);
			}

			function question_state_changed()
			{
				if (xmlHttp.readyState==4 && xmlHttp.status==200)
				{ 		  
				  var q_no = curr_id;
				  q_no++;
				  
				  var nextdiv = document.getElementById("q"+q_no);
				  var response = xmlHttp.responseText;
				  nextdiv.innerHTML=response;
				  
				  //create another question DIV after this one ready to store the next answer.
				  q_no++;
				  var divIdName = "q"+q_no;
				  var newdiv = document.createElement('div');
				  newdiv.setAttribute("id",divIdName);
				  nextdiv.appendChild(newdiv);								
				}
			}
			
			function calculate_points(question)
			{
				var points = 0;
				var q_no = curr_id;
				
				var points_element = document.getElementById("q"+question+"_points");
				var point_val = 0;
				if (points_element != null) point_val = points_element.value;
				accum_points[q_no] = parseFloat(point_val);
				
				
				for (var i = 1; i <= q_no; i++) {
					points = points + accum_points[i];
				}
				
				return points;
			}

			function reset_questions()
			{
				get_next_step("<?php echo $default_val; ?>", <?php echo $question->id; ?>);
				var first_q = document.getElementById("<?php echo "q".$question->id; ?>");
				if (first_q.type == "radio") {
					var radio_boxes = document.getElementsByName(first_q.name);
					for (var e = 0; e < radio_boxes.length; e++) {
						if (radio_boxes[e].type == "radio") {
							radio_boxes[e].checked = false;
						}
					}
				} else if (first_q.type == "select-one") {
					first_q.selectedIndex = "<?php echo $default_val; ?>";
				} else if (first_q.type == "text") {
					first_q.value = "";
				}
			}

			function decision_tree_validate_numeric(answer)
			{
				answer.replace(/,/, "");
				return answer.match(<?php echo $regex_code ?>);

			}

		</script>
		<?php
		$contents = ob_get_contents();
		ob_end_clean();

		return $contents;
		
	}//end generateJS()


	/**
	* Creates a new type format for the type_code specified by $new_type
	*
	* @param string	$new_type	the type_code of the type format to create
	*
	* @return boolean
	* @access private
	*/
	private function _createTypeFormat($new_type)
	{
		$edit_fns = $this->getEditFns();
		$folder   = $this->getFolder('type_formats');
		$types = $GLOBALS['SQ_SYSTEM']->am->getTypeDescendants('decision_tree_question');
		
		// check that the new type is valid
		if ($new_type != '' && !in_array($new_type, $types)) {
			trigger_localised_error('CMS0045', E_USER_WARNING, $new_type);
			return FALSE;
		}

		// check that this type isnt already customised
		$current_link = $GLOBALS['SQ_SYSTEM']->am->getLink($folder->id, SQ_LINK_TYPE_2, 'bodycopy', TRUE, $new_type);
		if (empty($current_link)) {
			// create a new format bodycopy for this type
			$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
			$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
			$GLOBALS['SQ_SYSTEM']->am->includeAsset('bodycopy');

			$copy_link = Array(
							'asset'			=> &$folder,
							'value'			=> $new_type,
							'link_type'		=> SQ_LINK_TYPE_2,
							'is_dependant'	=> 1,
							'is_exclusive'	=> 1,
						 );

			$bodycopy = new Bodycopy();
			$bodycopy->setAttrValue('name', $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($new_type, 'name').' Format');
			$args = Array('content' => $this->_getDefaultBodycopyContent($new_type));
			if (!$bodycopy->create($copy_link, $args)) {
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
				return FALSE;
			}
			$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
		} else {
			trigger_localised_error('CMS0046', E_USER_WARNING, $new_type);
			return FALSE;
		}

		return TRUE;

	}//end _createTypeFormat()
	
	
	/**
	* Return a reference to a folder linked to this asset determined by the passed in value on the link
	*
	* @param string	$type	the value on the link for this folder
	*
	* @return object
	* @access public
	*/
	public function &getFolder($type='type_formats')
	{
		$null = NULL;
		$link = $GLOBALS['SQ_SYSTEM']->am->getLink($this->id, SQ_LINK_TYPE_2, 'folder', TRUE, $type);
		if (empty($link)) return $null;

		$folder = $GLOBALS['SQ_SYSTEM']->am->getAsset($link['minorid'], $link['minor_type_code']);
		if (is_null($folder)) return $null;

		return $folder;

	}//end getFolder()
	
	
	/**
	* Get the default content for a bodycopy
	*
	* @param string	$bodycopy_code	The code name for the bodycopy
	*
	* @return string
	* @access private
	*/
	private function _getDefaultBodycopyContent($bodycopy_code)
	{
		switch ($bodycopy_code) {
			case 'page_contents':
				return '%decision_tree_contents% <br /> %reset_button%';
			case 'decision_tree_question_type_select':
				return '%question_title% <br /> %question_field%';
			case 'decision_tree_question_type_numeric':
				return '%question_title% <br /> %question_field% &nbsp; %question_button%';
		}
		return '';

	}//end _getDefaultBodycopyContent()
	
	
	/**
	* returns a reference to a bodycopy based of the name of the bodycopy
	* and the type (i.e the folder is type_formats or result_bodycopies)
	*
	* @param string	$name	the name of the bodycopy
	* @param string	$type	the type of the bodycopy
	*
	* @return object
	* @access public
	*/
	public function &getBodycopy($name='', $type='')
	{
		$null = NULL;

		if (!$name || !$type) return $null;
		$am = $GLOBALS['SQ_SYSTEM']->am;

		$folder = $this->getFolder($type);
		if (is_null($folder)) return $null;
		
		$bodycopy_link = $GLOBALS['SQ_SYSTEM']->am->getLink($folder->id, SQ_LINK_TYPE_2, 'bodycopy', TRUE, $name);
		if (empty($bodycopy_link)) return $null;

		$bodycopy = $am->getAsset($bodycopy_link['minorid'], $bodycopy_link['minor_type_code']);

		return $bodycopy;

	}//end getBodycopy()
	
	
	/*
	* Makes a call to getLinks() function to get all bodycopyies under the Result Bodycopies folder.
	* Mainly used it Page_Decision_Tree_Edit_Fns.
	*
	* @return Array
	* @access public
	*/
	public function getResultBodycopies()
	{
		
		$folder = $this->getFolder('result_bodycopies');
		$bodycopies = (!empty($folder)) ? $GLOBALS['SQ_SYSTEM']->am->getLinks($folder->id, SQ_LINK_TYPE_2, 'bodycopy', TRUE) : Array();	
			
		return $bodycopies;
		
	}//end getResultBodycopies()
	
	
	/**
	* Creates bodycopies for this asset under the Result Bodycopies folder.
	* 
	* @param string	$name		the name of the bodycopy
	* @param string	$link_value	the required link value
	*
	* @return boolean
	* @access protected
	*/
	public function createResultBodycopy($name, $link_value)
	{
		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
		
		$GLOBALS['SQ_SYSTEM']->am->includeAsset('bodycopy');
		$asset = new Bodycopy();
		$parent = $this->getFolder('result_bodycopies');
		if (is_null($parent)) return FALSE;
		
		$copy_link = Array('asset' => $parent, 'value' => $link_value ,'link_type' => SQ_LINK_TYPE_2, 'is_dependant' => 1, 'is_exclusive' => 1);
		$asset->setAttrValue('name', $name);
		if (!$asset->create($copy_link)) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			return FALSE;
		}
		
		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
		
		return TRUE;

	}//end createResultBodycopy()
	
	
	/**
	* Get a replacement for a keyword used in the type format bodycopies
	*
	* If no suitable replacement exists, the function will return NULL instead.
	*
	* @param string	$keyword	The keyword to replace
	*
	* @return mixed string|NULL
	*/
	public function getTypeFormatKeywordReplacement($keyword, $question_asset)
	{
		
		$replacement = NULL;
		if (empty($question_asset)) return $replacement;
		$is_question_keyword = preg_match('/^question\_(.*)$/', $keyword, $question_matches);

		if ($is_question_keyword) {
			// Sub keyword in $matches[1] - eg. 'field', 'note', 'label'
			list(, $sub_keyword) = $question_matches;
			switch ($sub_keyword) {
				case 'field' :
					$replacement = $question_asset->getHtmlField();
				break;
	
				case 'title' :
					$replacement = $question_asset->attr('name');
				break;
	
				case 'note' :
					$replacement = $question_asset->attr('note');
				break;
				
				case 'button' :
					if ($question_asset->type() != 'decision_tree_question_type_numeric') break;
					$replacement = $question_asset->getHtmlButton();
				break;
			}//end switch
		} 

		return $replacement;

	}//end getTypeFormatKeywordReplacement()
	
	
	/**
	* Get a replacement for a keyword used in the Page Contents of this asset
	*
	* If no suitable replacement exists, the function will return NULL instead.
	*
	* @param string	$keyword	The keyword to replace
	*
	* @return mixed string|NULL
	*/
	public function getPageContentsKeywordReplacement($keyword){
		
		$replacement = NULL;
		$replacements = Array();
		
		if ($keyword == 'reset_button') {
			ob_start();
			reset_button('qReset', 'Reset', 'reset_questions()');
			$replacement = ob_get_contents();
			ob_end_clean();	
			return $replacement;
		}
		
		if ($keyword != 'decision_tree_contents') return $replacement;
		
		$first_question = $GLOBALS['SQ_SYSTEM']->am->getLink($this->id, SQ_LINK_NOTICE, 'decision_tree_question', FALSE, 'first_question');
		if (empty($first_question)) {
			trigger_localised_error('CMS0118', E_USER_NOTICE, $this->id);	
			return $replacement;
		}

		$question = $GLOBALS['SQ_SYSTEM']->am->getAsset($first_question['minorid']);
		
		$bodycopy = $this->getBodycopy($question->type(), 'type_formats');
		// check to see if the bodycopy attached to this form has any content
		if ($bodycopy && !$bodycopy->isEmpty()) {
			$keyword_list = $bodycopy->getKeywords();
			
			if (!empty($keyword_list)) {
				foreach ($keyword_list as $keyword) {
					$replacement = NULL;
					if ($keyword == 'accumulated_points'){
						$replacements[$keyword] = 0;
						continue;
					}
					// Question related keywords are found here
					$replacement = $this->getTypeFormatKeywordReplacement($keyword, $question);
					
					if (is_null($replacement)) {
						// If not picked up by first function, this will pick up
						// replacements placed in separate functions - CAPTCHA,
						// errors, for instance
						$replacement = $this->getKeywordReplacement($keyword);
					}

					if (!is_null($replacement)) {
						// We have something from one of the above
						$replacements[$keyword] = $replacement;
					}
				}
			}

			$bodycopy->setKeywordReplacements($replacements);
			ob_start();
			echo '<div id="questions">';
			echo '<div id="q1">';
			echo '<div id="q'.$first_question['minorid'].'_wrapper" class="sq_question_wrapper">';
			$bodycopy->printBody();
			echo '</div>';
			echo '<div id="q2">';
			echo '</div>';
			echo '</div>';
			echo '</div>';
			$ajax = $this->generateJS($question);
			echo $ajax;
			$replacement = ob_get_contents();
			
			ob_end_clean();
		} 
		
		return $replacement;
		
	}//end getPageContentsKeywordReplacement()
	
	//--        KEYWORDS AVAILABLE        --//


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
		
		if (!isset($vars['keywords'])) return;

		$keywords = Array();
		
		$parents = $GLOBALS['SQ_SYSTEM']->am->getParents($broadcaster->id, 'bodycopy', TRUE);
		$page_contents = $GLOBALS['SQ_SYSTEM']->am->getLink($this->id, SQ_LINK_TYPE_2, 'bodycopy', TRUE, 'page_contents', 'major', TRUE);
		if (empty($page_contents)) return FALSE;

		//for the keywords of the Page Contents
		if (array_key_exists($page_contents['minorid'], $parents)) {
			$keywords = @$this->_tmp['keywords']['page_contents'];
			if (is_null($keywords) || !is_array($keywords)) {
				$keywords = $this->_getPageContentsBodycopyKeywords();
				$this->_tmp['keywords']['page_contents'] = $keywords;
			}
		} else {
			//else for the keywords of the Type Format bodycopies
			
			$folder = $this->getFolder('type_formats');
			if (is_null($folder)) return FALSE;
	
			// lets work out which bodycopy the WYSIWYG cell that wants our keywords
			// is in so we can return the correct keywords for the screen
			$bodycopy_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($folder->id, SQ_LINK_TYPE_2, 'bodycopy');
			
			$bodycopy_code = NULL;
			foreach ($bodycopy_links as $link_info) {
				if (isset($parents[$link_info['minorid']])) {
					$bodycopy_code = $link_info['value'];
					break;
				}
			}
	
			if (is_null($bodycopy_code)) return;
			
			$keywords = @$this->_tmp['keywords'][$bodycopy_code];
			
			if (is_null($keywords) || !is_array($keywords)) {
				$keywords = Array();
				// note: if the value on the link is "decision_tree_question_type_select", the method providing the keywords should be _getQuestionTypeSelectBodycopyKeywords
				
				$keyword_provider_method = str_replace('decision_tree_', '', $bodycopy_code);
				$keyword_provider_method = str_replace('_', ' ', $keyword_provider_method);
				$keyword_provider_method = '_get'.str_replace(' ', '', ucwords($keyword_provider_method)).'BodycopyKeywords';
				
				if (method_exists($this, $keyword_provider_method)) {
					$keywords = $this->$keyword_provider_method();
					$this->_tmp['keywords'][$bodycopy_code] = $keywords;
				}
				
			}
		}
		$vars['keywords'] = array_merge($vars['keywords'], $keywords);

	}//end onRequestKeywords()

	
	/**
	* Get an array of keywords for Question Type Select Bodycopy
	*
	* @return array
	* @access private
	*/
	private function _getQuestionTypeSelectBodycopyKeywords()
	{
		
		$keywords['question_title'] = 'Question Title';
		$keywords['question_field'] = 'Question Input Field';
		$keywords['question_note'] = 'Question Note';
		$keywords['accumulated_points'] = 'Accumulated Points';
		
		return $keywords;
		
	}//end _getQuestionTypeSelectBodycopyKeywords()
	
	
	/**
	* Get an array of keywords for Question Type Numeric Bodycopy
	*
	* @return array
	* @access private
	*/
	private function _getQuestionTypeNumericBodycopyKeywords()
	{
		
		$keywords['question_title'] = 'Question Title';
		$keywords['question_field'] = 'Question Input Field';
		$keywords['question_note'] = 'Question Note';
		$keywords['question_button'] = 'Question Button';
		$keywords['accumulated_points'] = 'Accumulated Points';
		
		return $keywords;
		
	}//end _getQuestionTypeNumericBodycopyKeywords()
	
	
	/**
	* Get an array of keywords for Page Contents Bodycopy
	*
	* @return array
	* @access private
	*/
	private function _getPageContentsBodycopyKeywords()
	{
		
		$keywords['decision_tree_contents']  = 'Decision Tree Contents';
		$keywords['reset_button'] = 'Reset Button';

		return $keywords;

	}//end _getPageContentsBodycopyKeywords()

}//end class

?>
