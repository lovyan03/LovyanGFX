# AI Contribution Policy

This policy applies to contributors of code to LovyanGFX open source project, as
well as reviewers of contributions.

Section 1 contains guidelines for contributors. Section 2 contains guidelines
for Reviewers.

# SECTION 1: For Contributors:

The LovyanGFX community welcomes the use of AI tools as part of your contribution
workflow.

AI can be genuinely useful: for learning a new part of the codebase, drafting
descriptions, thinking through a problem, or keeping track of a complex review.
The goal of this policy is not to restrict AI use, but to ensure that
contributions reflect real understanding and that AI use is transparent.

Two principles underpin everything here:

* **Understanding**: You are responsible for what you submit. Using AI to
  generate code or text you don't understand is not a shortcut; it shifts work
  onto reviewers and maintainers without adding value. Using AI to *build*
  understanding is encouraged.  
* **Transparency**: "Is this AI?" is not a fruitful question standing alone.
  Disclose AI use clearly, so reviewers can calibrate their expectations and
  everyone stays on the same page.

**A Note on Quality**

The goal of this policy is not to track AI use for its own sake. It is to
maintain the quality and reviewability of contributions to a large, complex
platform that many institutions and learners depend on. Unnecessary code churn,
bypassed tests, and unreviewed AI output all have real downstream costs.

You can use AI extensively and produce excellent contributions. You can also
produce low-quality contributions without using AI at all. What matters is
understanding, care, and transparency, regardless of the tools involved.

## Pull requests

### Contributor requirements

* **Disclose AI use explicitly.** If AI tools contributed meaningfully to your
  PR (code, tests, or description), say so in the PR description. Use [GitHub's
  co-author
  feature](https://docs.github.com/en/pull-requests/committing-changes-to-your-project/creating-and-editing-commits/creating-a-commit-with-multiple-authors)
  where applicable (e.g., `Co-authored-by: Claude <claude@anthropic.com>`).
  Please consider sharing your AI conversation log directly in the PR
  description, or link to the log in a GitHub Gist. There is no penalty for
  disclosure; there is a problem with concealment.

* **Understand the code you are submitting.** Before opening a [non-draft
  PR](https://docs.github.com/en/pull-requests/collaborating-with-pull-requests/proposing-changes-to-your-work-with-pull-requests/about-pull-requests#draft-pull-requests),
  you must be able to explain what the proposed changes do and why they are
  correct. If a reviewer asks you about your implementation and your honest
  answer is "I'm not sure, the AI did it," the PR is not ready, and your PR will
  likely be closed.

* **Review all AI-generated output before submitting.** You are responsible for
  all code or other material you submit, including its quality and
  appropriateness for contribution. 

* **Verify the accuracy of AI-generated material.** This includes everything you
  contribute: PR descriptions, inline comments, and responses to reviewer
  feedback. If an AI tells you that review feedback has been addressed, verify
  that it actually has been. Do not relay AI output as your own assessment.

* **Feel free to use AI to improve documentation**. When you submit a PR,
  consider using AI to help determine what corresponding PRs might apply to
  documentation. However, you should apply the same quality review and human
  curation to documentation that you would to code PRs.

## Issue reports and triage

The same principles apply to issue reports:

1. **Disclose AI use.** If AI helped you write or structure the issue, say so.  
2. **Understand what you are filing.** AI-generated issue reports often lack the
   specific reproduction steps, environmental context, and genuine debugging
   effort that make issues actionable. Do not file an issue you could not
   discuss or expand on if asked.

Maintainers may close issues that appear fully AI-generated without legitimate
investigation. Contributors who repeatedly open unproductive issues may be
blocked.

## Acceptable uses

AI tools are well-suited to:

* Familiarizing yourself with an unfamiliar part of the codebase  
* Drafting PR descriptions or issue reports (with human review and editing)  
* Assisting with writing in a non-native language  
* Talking through a design decision or debugging approach  
* Assisting reviewers in understanding a large or complex diff

These are uses that support human understanding and judgment, not a substitute
for it.

---

### ⚠️ Common AI Tool Antipatterns

Be on the lookout for common AI tool patterns that lower the quality of your
contribution. AI tools commonly:

* Rewrite entire code blocks when a small, focused edit was needed, making diffs
  harder to review and introducing unintended changes.  
* Fix failing tests by altering or bypassing them rather than addressing the
  underlying problem.  
* Generate comments that describe what the code does rather than why, adding
  noise without value.

It is your responsibility to catch and correct these issues before requesting
review.

---

### ❌ Bad flow: no human understanding anywhere in the loop

1. Contributor prompts an LLM with a link to a GitHub issue: "Fix this and open
   a PR."  
2. Reviewer prompts an LLM with a link to the PR: "Review this."  
3. LLM responds "LGTM." PR is merged.

Result: code enters the codebase that no human has read or understood.

---

### ❌ Bad flow: review loop with no real engagement

1. Contributor prompts an LLM with a link to a GitHub issue: "Fix this and open
   a PR."  
2. Reviewer reviews and leaves comments.  
3. Contributor prompts an LLM with a link to the review: "Address this
   feedback."  
4. Repeat indefinitely.

Result: the reviewer is effectively collaborating with a very high-latency LLM.
No matter how many rounds this takes, it is not a productive review process. If
you recognize this pattern, close the PR.

---

### ✅ Good flow: AI assists, humans understand

1. Contributor prompts an LLM with context about the issue.  
2. Contributor reviews the output, makes sure they understand the changes,
   adjusts as needed, and submits the PR with appropriate disclosure.  
3. Reviewer uses an LLM to help understand the diff or organize their thoughts.  
4. Reviewer verifies they understand the changes and any suggestions before
   writing their review.  
5. Reviewer comments. Contributor engages with the feedback directly, using
   their own understanding of the code.

Result: AI accelerates the work. Humans remain responsible for it.

# SECTION 2: For Reviewers

Reviewer time is limited. If you are a reviewer, you are not obligated to
continue reviewing a PR that shows signs the contributor has not followed this
policy. Consider closing PRs that show any of the following:

* The PR description does not match the changes (the contributor did not review
  both carefully enough to notice).  
* The PR contains irrelevant AI-generated files (e.g., agent planning notes,
  scaffolding files).  
* Review responses do not engage with feedback; they restate or reformat rather
  than address it.  
* The contributor cannot explain their own changes when asked directly.

**Beware the sunk cost fallacy.** If at any point you feel you are effectively
acting as a high-latency prompt engineer (feeding feedback into a loop that
returns minimally filtered AI output), it is appropriate to close the PR and
explain why. This applies even after multiple review rounds.

You may also use AI tools to assist your review. If you do, disclose it, for
example:

*Review assisted by Claude. I used it to talk through the changes, track my
notes, and help draft this summary. All conclusions are my own.*


This AI Policy document was inspired by [openedx AI_POLICY.md](https://github.com/openedx/.github/blob/master/AI_POLICY.md).
