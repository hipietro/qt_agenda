// Search implementation. Normalization is kept here so the GUI stays simple.

#include "SearchEngine.h"

#include <QChar>
#include <QStringList>
#include <QVector>

#include <algorithm>
#include <limits>

bool SearchEngine::SearchSuggestion::isValid() const
{
    return activity != nullptr && !suggestedText.isEmpty() && distance >= 0;
}

bool SearchEngine::SearchResponse::hasResults() const
{
    return !results.empty();
}

SearchEngine::SearchResponse SearchEngine::search(const std::vector<const Activity*>& activities,
                                                   const QString& query)
{
    SearchResponse response;

    const QString normalizedQuery = normalize(query);

    if (normalizedQuery.isEmpty()) {
        for (const Activity* activity : activities) {
            if (activity) {
                response.results.push_back({activity, 0, "all"});
            }
        }

        return response;
    }

    for (const Activity* activity : activities) {
        if (!activity) {
            continue;
        }

        int bestScore = 0;
        QString matchedField;

        const int titleScore = calculateFieldScore(activity->title(),
                                                   normalizedQuery,
                                                   1000,
                                                   850,
                                                   700);
        if (titleScore > bestScore) {
            bestScore = titleScore;
            matchedField = "title";
        }

        const int categoryScore = calculateFieldScore(activity->category(),
                                                      normalizedQuery,
                                                      700,
                                                      550,
                                                      450);
        if (categoryScore > bestScore) {
            bestScore = categoryScore;
            matchedField = "category";
        }

        const int descriptionScore = calculateFieldScore(activity->description(),
                                                         normalizedQuery,
                                                         450,
                                                         350,
                                                         250);
        if (descriptionScore > bestScore) {
            bestScore = descriptionScore;
            matchedField = "description";
        }

        const int summaryScore = calculateFieldScore(activity->summary(),
                                                     normalizedQuery,
                                                     400,
                                                     300,
                                                     200);
        if (summaryScore > bestScore) {
            bestScore = summaryScore;
            matchedField = "summary";
        }

        if (bestScore > 0) {
            response.results.push_back({activity, bestScore, matchedField});
        }
    }

    std::sort(response.results.begin(), response.results.end(),
              [](const SearchResult& first, const SearchResult& second) {
                  if (first.score != second.score) {
                      return first.score > second.score;
                  }

                  return first.activity->title().toLower() < second.activity->title().toLower();
              });

    if (response.results.empty()) {
        response.suggestion = findBestSuggestion(activities, normalizedQuery);
    }

    return response;
}

QString SearchEngine::normalize(const QString& text)
{
    const QString decomposed = text.normalized(QString::NormalizationForm_D)
                                   .toLower()
                                   .simplified();

    QString result;

    for (const QChar& character : decomposed) {
        const QChar::Category category = character.category();

        if (category == QChar::Mark_NonSpacing ||
            category == QChar::Mark_SpacingCombining ||
            category == QChar::Mark_Enclosing) {
            continue;
        }

        result.append(character);
    }

    return result.simplified();
}

int SearchEngine::calculateFieldScore(const QString& fieldText,
                                      const QString& normalizedQuery,
                                      int exactScore,
                                      int startsWithScore,
                                      int containsScore)
{
    const QString normalizedField = normalize(fieldText);

    if (normalizedField.isEmpty()) {
        return 0;
    }

    if (normalizedField == normalizedQuery) {
        return exactScore;
    }

    if (normalizedField.startsWith(normalizedQuery)) {
        return startsWithScore;
    }

    if (normalizedField.contains(normalizedQuery)) {
        return containsScore;
    }

    return 0;
}

int SearchEngine::levenshteinDistance(const QString& first,
                                      const QString& second)
{
    if (first == second) {
        return 0;
    }

    if (first.isEmpty()) {
        return second.length();
    }

    if (second.isEmpty()) {
        return first.length();
    }

    QVector<int> previousRow(second.length() + 1);
    QVector<int> currentRow(second.length() + 1);

    for (int j = 0; j <= second.length(); ++j) {
        previousRow[j] = j;
    }

    for (int i = 1; i <= first.length(); ++i) {
        currentRow[0] = i;

        for (int j = 1; j <= second.length(); ++j) {
            const int insertionCost = currentRow[j - 1] + 1;
            const int deletionCost = previousRow[j] + 1;
            const int substitutionCost = previousRow[j - 1] + (first[i - 1] == second[j - 1] ? 0 : 1);

            currentRow[j] = std::min({insertionCost, deletionCost, substitutionCost});
        }

        previousRow = currentRow;
    }

    return previousRow[second.length()];
}

SearchEngine::SearchSuggestion SearchEngine::findBestSuggestion(const std::vector<const Activity*>& activities,
                                                                const QString& normalizedQuery)
{
    SearchSuggestion bestSuggestion;

    int bestDistance = std::numeric_limits<int>::max();
    const int acceptedDistance = maxAcceptedDistance(normalizedQuery.length());

    auto evaluateCandidate = [&](const Activity* activity, const QString& candidateText) {
        const QString normalizedCandidate = normalize(candidateText);

        if (normalizedCandidate.isEmpty()) {
            return;
        }

        QStringList candidates;
        candidates.append(normalizedCandidate);

        const QStringList words = normalizedCandidate.split(' ', Qt::SkipEmptyParts);
        for (const QString& word : words) {
            if (word.length() >= 3) {
                candidates.append(word);
            }
        }

        for (const QString& candidate : candidates) {
            const int distance = levenshteinDistance(normalizedQuery, candidate);

            if (distance < bestDistance) {
                bestDistance = distance;
                bestSuggestion.activity = activity;
                bestSuggestion.suggestedText = activity->title();
                bestSuggestion.matchedText = candidate;
                bestSuggestion.distance = distance;
            }
        }
    };

    for (const Activity* activity : activities) {
        if (!activity) {
            continue;
        }

        evaluateCandidate(activity, activity->title());
        evaluateCandidate(activity, activity->category());
        evaluateCandidate(activity, activity->description());
        evaluateCandidate(activity, activity->summary());
    }

    if (bestDistance > acceptedDistance) {
        return SearchSuggestion{};
    }

    return bestSuggestion;
}

int SearchEngine::maxAcceptedDistance(int queryLength)
{
    if (queryLength <= 4) {
        return 1;
    }

    if (queryLength <= 7) {
        return 2;
    }

    return 3;
}