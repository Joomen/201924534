package main

import (
    "encoding/json"
    "strconv"
    "strings"

    "github.com/hyperledger/fabric-contract-api-go/contractapi"
)

// Asset 구조체 정의
type Asset struct {
    ID                                    string  `json:"id"`
    SignificantStrikesAbsorbedPerMinute   float64 `json:"significant_strikes_absorbed_per_minute"`
    SignificantStrikeDefence              float64 `json:"significant_strike_defence"`
    AverageTakedownsLandedPer15Minutes    float64 `json:"average_takedowns_landed_per_15_minutes"`
    TakedownAccuracy                       float64 `json:"takedown_accuracy"`
    TakedownDefense                        float64 `json:"takedown_defense"`
    AverageSubmissionsAttemptedPer15Minutes float64 `json:"average_submissions_attempted_per_15_minutes"`
}

// SmartContract 구조체 정의
type SmartContract struct {
    contractapi.Contract
}

// CreateAsset 메서드: 새로운 에셋 생성
func (s *SmartContract) CreateAsset(ctx contractapi.TransactionContextInterface, id string, significantStrikesAbsorbedPerMinute float64,
    significantStrikeDefence float64, averageTakedownsLandedPer15Minutes float64, takedownAccuracy float64,
    takedownDefense float64, averageSubmissionsAttemptedPer15Minutes float64) error {
    
    asset := Asset{
        ID:                                    id,
        SignificantStrikesAbsorbedPerMinute:   significantStrikesAbsorbedPerMinute,
        SignificantStrikeDefence:              significantStrikeDefence,
        AverageTakedownsLandedPer15Minutes:    averageTakedownsLandedPer15Minutes,
        TakedownAccuracy:                       takedownAccuracy,
        TakedownDefense:                        takedownDefense,
        AverageSubmissionsAttemptedPer15Minutes: averageSubmissionsAttemptedPer15Minutes,
    }

    assetJSON, err := json.Marshal(asset)
    if err != nil {
        return err
    }

    return ctx.GetStub().PutState(id, assetJSON)
}

// ReadAsset 메서드: 특정 에셋 조회
func (s *SmartContract) ReadAsset(ctx contractapi.TransactionContextInterface, id string) (*Asset, error) {
    assetJSON, err := ctx.GetStub().GetState(id)
    if err != nil {
        return nil, err
    }
    if assetJSON == nil {
        return nil, nil // 에셋이 존재하지 않는 경우
    }

    var asset Asset
    err = json.Unmarshal(assetJSON, &asset)
    if err != nil {
        return nil, err
    }

    return &asset, nil
}

// UpdateAsset 메서드: 특정 에셋 업데이트
func (s *SmartContract) UpdateAsset(ctx contractapi.TransactionContextInterface, id string, significantStrikesAbsorbedPerMinute float64,
    significantStrikeDefence float64, averageTakedownsLandedPer15Minutes float64, takedownAccuracy float64,
    takedownDefense float64, averageSubmissionsAttemptedPer15Minutes float64) error {

    existingAsset, err := s.ReadAsset(ctx, id)
    if err != nil {
        return err
    }
    if existingAsset == nil {
        return fmt.Errorf("asset with ID %s does not exist", id)
    }

    // 기존 에셋 업데이트
    existingAsset.SignificantStrikesAbsorbedPerMinute = significantStrikesAbsorbedPerMinute
    existingAsset.SignificantStrikeDefence = significantStrikeDefence
    existingAsset.AverageTakedownsLandedPer15Minutes = averageTakedownsLandedPer15Minutes
    existingAsset.TakedownAccuracy = takedownAccuracy
    existingAsset.TakedownDefense = takedownDefense
    existingAsset.AverageSubmissionsAttemptedPer15Minutes = averageSubmissionsAttemptedPer15Minutes

    assetJSON, err := json.Marshal(existingAsset)
    if err != nil {
        return err
    }

    return ctx.GetStub().PutState(id, assetJSON)
}

// DeleteAsset 메서드: 특정 에셋 삭제
func (s *SmartContract) DeleteAsset(ctx contractapi.TransactionContextInterface, id string) error {
    return ctx.GetStub().DelState(id)
}

// GetAllAssets 메서드: 모든 에셋 조회
func (s *SmartContract) GetAllAssets(ctx contractapi.TransactionContextInterface) ([]Asset, error) {
    resultsIterator, err := ctx.GetStub().GetStateByRange("", "")
    if err != nil {
        return nil, err
    }
    defer resultsIterator.Close()

    var assets []Asset
    for resultsIterator.HasNext() {
        queryResponse, err := resultsIterator.Next()
        if err != nil {
            return nil, err
        }

        var asset Asset
        err = json.Unmarshal(queryResponse.Value, &asset)
        if err != nil {
            return nil, err
        }
        assets = append(assets, asset)
    }
    return assets, nil
}

// 메인 함수
func main() {
    chaincode, err := contractapi.NewChaincode(new(SmartContract))
    if err != nil {
        panic(err)
    }
    if err := chaincode.Start(); err != nil {
        panic(err)
    }
}
